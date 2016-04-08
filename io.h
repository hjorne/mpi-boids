#ifndef _IO_H_
#define _IO_H_

#include "boid.h"

/* All input parameters of a simulation */
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

/* Declare a default config */
Config* DefaultConfig(void);

/* Reads in config */
Config* ReadConfig(char*);

/* Turn char** into char* */
char* ConcatenateOutput(char**, int, int);

/* Generate lines of output to be written */
char* GenerateRankData(Boid*, int, int, int, int, int*);

/* Write actual data */
void WriteRankData(char*, Boid*, int, int, int, int, int);

/* Handler function required bio ini library. See github for more info */
int handler(void* user, const char* section, const char* name, const char* value);

#endif
