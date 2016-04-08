#ifndef _INIT_H_
#define _INIT_H_

#include "vec.h"
#include "boid.h"
#include "io.h"

void Initialize(Boid**, Config*, int*, int, int);
void InitializeRanks(Boid**, int*, int, int, double);
int* DistributeBoids(Vec*, int, int, double);
Vec* InitBoidPositions(int, int, double);
int* BoidRanks(Vec*, int, int, double);
int VecToRank(Vec, double, int);
void CheckRanks(int, int);


#endif
