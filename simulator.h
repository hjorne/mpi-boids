#ifndef _SIMULATOR_H_
#define _SIMULATOR_H_

#include "boid.h"

// TODO: high-level overview of function decs in header?

void InitializeSim(Boid*, int, int, int, double);
void Iterate(int);
void Neighbors(int**, int*);
int* SendRecvNumBoids(int*, int, int);
Boid* SendRecvBoids(int*, int*, int, int);
int TotalNeighborBoids(int*, int);
Boid* ConcatenateBoids(Boid*, int);
void UpdateBoids(Boid*, int);

int mod(int, int);

double xGrid();
double yGrid();
int xQuad();
int yQuad();
double xMin();
double xMax();
double yMin();
double yMax();
int QuadToRank(int, int);

#endif
