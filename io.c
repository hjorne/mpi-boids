#include "io.h"
#include "ini.h"
#include <stdlib.h>

#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0

Config* DefaultConfig()
{
    Config* c = (Config*) malloc(sizeof(Config));

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


static int handler(void* user, const char* section, const char* name, const char* value)
{
    Config* pconfig = (Config*)user;

    if (MATCH("", "seed")) {
        pconfig->seed = atoi(value);
    } else if (MATCH("", "numboids")) {
        pconfig->numboids = atoi(value);
    } else if (MATCH("", "numticks")) {
        pconfig->numticks = atoi(value);
    } else if (MATCH("", "v")) {
        pconfig->v = atof(value);
    } else if (MATCH("", "noise")) {
        pconfig->noise = atof(value);
    } else if (MATCH("", "cutoff")) {
        pconfig->cutoff = atof(value);
    } else if (MATCH("", "sidelen")) {
        pconfig->sidelen = atof(value);
    } else if (MATCH("", "dt")) {
        pconfig->dt = atof(value);
    }
    else {
        return 0;  /* unknown section/name, error */
    }
    return 1;
}


void WriteRankData(Boid* boids, int mynumboids, int ticknum)
{
    char fname[80];

}
