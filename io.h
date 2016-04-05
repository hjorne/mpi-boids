#ifndef _IO_H_
#define _IO_H_

#include "boid.h"

typedef struct config_s {
    char* fname;
    int seed;
    int numboids;
    int numticks;
    double v;
    double dt;
    double noise;
    double cutoff;
    double sidelen;
} Config;

Config* DefaultConfig();
Config* ReadConfig(char*);
void WriteRankData(char*, Boid*, int, int, int, int, int);
char* GenerateRankData(Boid*, int, int, int, int, int*);
char* ConcatenateOutput(char**, int, int);
int handler(void* user, const char* section, const char* name, const char* value);

#endif
