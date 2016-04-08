#include "io.h"
#include "ini.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdio.h>
#include <mpi.h>

/* Define macro specified in ini example. See github */
#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0

/*
 * Controller function for output. Uses a static variable (so that it stays persistent between
 * calls) to keep track of a global offset within the file, so that each timestep doesn't overwrite
 * another, and calculates a local offset based of how many bytes each rank is writing. Finds this
 * with an MPI_Allgather each call
 */
void
WriteRankData(char* fname, Boid* boids, int mynumboids, int global_numboids, int ticknum,
              int myrank, int numranks)
{

    static int global_offset;
    int num_bytes, i;
    int total_bytes = 0;
    int local_offset = 0;
    MPI_File fh;

    char* io_line = GenerateRankData(boids, myrank, mynumboids, global_numboids, ticknum,
                                     &num_bytes);
    int* bytes_per_rank = (int*) calloc(numranks, sizeof(int));

    MPI_Allgather(&num_bytes, 1, MPI_INT, bytes_per_rank, 1, MPI_INT, MPI_COMM_WORLD);

    for (i = 0; i < numranks; ++i) {
        total_bytes += bytes_per_rank[i];
        if (myrank > i)
            local_offset += bytes_per_rank[i];
    }

    MPI_File_open(MPI_COMM_WORLD, fname, MPI_MODE_CREATE | MPI_MODE_WRONLY, MPI_INFO_NULL, &fh);
    MPI_File_write_at(fh, global_offset + local_offset, io_line, num_bytes, MPI_CHAR,
                      MPI_STATUS_IGNORE);
    MPI_File_close(&fh);

    global_offset += total_bytes;

    free(bytes_per_rank);
    free(io_line);
}

/*
 * Generates output line for each boid in simulation. Uses sprintf to write strings to a large
 * buffer array in the stack. Keeps track of the number of bytes written usen return values from
 * sprintf. Rank 0 also writes the 'headers' for each timestep, along with controlling the newlines
 * between each timestep so everything looks nice
 */
char*
GenerateRankData(Boid* boids, int myrank, int mynumboids, int global_numboids, int ticknum,
                 int* num_bytes)
{
    Boid b;
    char buff[1024];
    int i, n_temp;
    int n = 0;
    int offset = 0;
    char** io_lines = NULL;

    /* Make sure headers and newlines are done appropriatiately. Different size callocs are needed
       depending on  whether or not headers are included or not*/
    if (myrank == 0) {
        io_lines = (char**) calloc(mynumboids + 1, sizeof(char*));

        if (ticknum > 0)
            n += sprintf(buff, "\n%i\n# Time step = %i\n", global_numboids, ticknum);
        else
            n += sprintf(buff, "%i\n# Time step = %i\n", global_numboids, ticknum);

        io_lines[0] = strdup(buff);
        offset = 1;
    }
    else {
        io_lines = (char**) calloc(mynumboids, sizeof(char*));
    }

    /* CHANGE ME FLAG
       If you need to change the format of each output line (include/exclude velocity info) change
       the sprintf line to your needs */
    for (i = 0; i < mynumboids; ++i) {
        b = boids[i];
        n += sprintf(buff, "%i %f %f 0.0 %f %f 0.0\n", b.id, b.r.x, b.r.y, b.v.x, b.v.y);
        io_lines[i + offset] = strdup(buff);
    }
    *num_bytes = n;

    /* This function actually generates an array of strings. Calls ConcatenateOutput so that the
       function can return 1 single coherent string per rank */
    return ConcatenateOutput(io_lines, mynumboids + offset, n);
}

/*
 * As stated above, this function returns a single string from an array of strings for easy writing
 * Handles null char manually by writing one to the end of the line. Does not include null chars
 * from any of the strings in the original array
 */
char* ConcatenateOutput(char** io_lines, int mynumboids, int num_chars)
{
    char* io_line = (char*) calloc(num_chars + 1, sizeof(char));

    int i, j, len;
    int k = 0;
    for (i = 0; i < mynumboids; ++i) {
        len = strlen(io_lines[i]);
        for (j = 0; j < len; ++j) {
            io_line[k++] = io_lines[i][j];
        }
        free(io_lines[i]);
    }
    io_line[k] = 0x00;
    free(io_lines);
    return io_line;
}

/*
 * Generates a default config. When the config is read, any option found will overwrite values
 * from here, but if a line corresponding to 'noise', for exapmle, is not found, a default value
 * is used. No guarantee that any of the default values here make sense.
 */
Config*
DefaultConfig(void)
{
    Config* c = (Config*) malloc(sizeof(Config));

    c->fname = "outfile.txt";
    c->seed = -1;  // -1 means random seed
    c->numboids = 30;
    c->numticks = 100;
    c->v = 0.03;
    c->dt = 1.0;
    c->noise = 1.0;
    c->cutoff = 1.0;
    c->sidelen = 5.0;

    return c;
}


Config* ReadConfig(char* fname)
{
    Config* c = DefaultConfig();

    if (ini_parse(fname, handler, c) < 0) {
        printf("Could not load config file %s\n", fname);
        exit(1);
    }

    return c;
}


int handler(void* user, const char* section, const char* name, const char* value)
{
    Config* pconfig = (Config*)user;

    if (MATCH("", "seed")) {
        pconfig->seed = atoi(value);
    }
    else if (MATCH("", "numboids")) {
        pconfig->numboids = atoi(value);
    }
    else if (MATCH("", "numticks")) {
        pconfig->numticks = atoi(value);
    }
    else if (MATCH("", "v")) {
        pconfig->v = atof(value);
    }
    else if (MATCH("", "noise")) {
        pconfig->noise = atof(value);
    }
    else if (MATCH("", "cutoff")) {
        pconfig->cutoff = atof(value);
    }
    else if (MATCH("", "sidelen")) {
        pconfig->sidelen = atof(value);
    }
    else if (MATCH("", "dt")) {
        pconfig->dt = atof(value);
    }
    else if (MATCH("", "filename")) {
        pconfig->fname = strdup(value);
    }
    else {
        return 0;  /* unknown section/name, error */
    }
    return 1;
}
