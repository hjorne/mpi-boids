#ifndef _INIT_H_
#define _INIT_H_

#include "vec.h"
#include "boid.h"
#include "io.h"

/* Makes sure the number of MPI ranks is valid */
void CheckRanks(int, int);

/* Finds the rank a given position is supposed to be at */
int VecToRank(Vec, double, int);

/* Create array where boid i goes to rank[i] */
int* BoidRanks(Vec*, int, int, double);

/* Intialize positions of all boids in simulation */
Vec* InitBoidPositions(int, int, double);

/* Break up boids before sending to ranks */
int* DistributeBoids(Vec*, int, int, double);

/* Initialize boids if rank 0, receive boids otherwise */
void Initialize(Boid**, Config*, int*, int, int);

/* Initialize velocities and actually send boids to necessary ranks */
void InitializeRanks(Boid**, int*, int, int, double);

#endif
