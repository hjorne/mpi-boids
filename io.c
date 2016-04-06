#include "io.h"
#include "ini.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <mpi.h>
#include <assert.h>
#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0

void WriteRankData(char* fname, Boid* boids, int mynumboids, int global_numboids, int ticknum,
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


char* GenerateRankData(Boid* boids, int myrank, int mynumboids, int global_numboids, int ticknum,
                       int* num_bytes)
{
    Boid b;
    char buff[1024];
    int i, n_temp;
    int n = 0;
    int offset = 0;
    char** io_lines = NULL;

    if (myrank == 0) {
        io_lines = (char**) calloc(mynumboids + 1, sizeof(char*));

        if (ticknum > 0)
            n += sprintf(buff, "\n%i\n#Time step%i\n", global_numboids, ticknum);
        else
            n += sprintf(buff, "%i\n#Time step%i\n", global_numboids, ticknum);

        io_lines[0] = strdup(buff);
        offset = 1;
    }
    else {
        io_lines = (char**) calloc(mynumboids, sizeof(char*));
    }

    for (i = 0; i < mynumboids; ++i) {
        b = boids[i];
        n += sprintf(buff, "bird%i %f %f 0.0 %f %f 0.0\n", b.id, b.r.x, b.r.y, b.v.x, b.v.y);
        io_lines[i + offset] = strdup(buff);
    }
    *num_bytes = n;
    return ConcatenateOutput(io_lines, mynumboids + offset, n);
}

char* ConcatenateOutput(char** io_lines, int mynumboids, int num_chars)
{
    char* io_line = (char*) calloc(num_chars + 1, sizeof(char));

    int i, j, len;
    for (i = 0; i < num_chars; ++i) {
        io_line[i] = 0x40;
    }
    int count = 0;
    for (i = 0; i < mynumboids; ++i) {
        len = strlen(io_lines[i]);
        for (j = 0; j < len; ++j) {
            io_line[count++] = io_lines[i][j];
        }
        free(io_lines[i]);
    }
    io_line[count] = 0x00;
    free(io_lines);
    return io_line;
}



Config* DefaultConfig()
{
    Config* c = (Config*) malloc(sizeof(Config));

    c->fname = "outfile.dat";
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
