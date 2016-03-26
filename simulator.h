#ifndef _SIMULATOR_H_
#define _SIMULATOR_H_

////////////////////////////////////////////////////////////////////////////////
#include "boid.h"
#include "io.h"
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// I know it's stupid, I can't help myself
void RecombineBoids(Boid**, int*, int*, int, int, int);
void InitializeSim(Boid*, Config*, int, int, int);
void RearrangeBoids(int*, int*, int*, int, int);
Boid* SendRecvBoids(int*, int*, int, int);
int CheckLocalBoundaries(double, double);
int* SendRecvNumBoids(int*, int, int);
void UpdatePosition(int*, int, int);
Boid* ConcatenateBoids(Boid*, int);
double AverageNormalizedVelocity();
int TotalNeighborBoids(int*, int);
void UpdateVelocity(Boid*, int);
void Neighbors(int**, int*);
int IndexOf(int*, int, int);
void PrintBoid(Boid);
void SanityCheck();
int mod(int, int);
void Iterate(int);
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Functions that should be inlined, but IBM's XL compiler won't let me
int QuadToRank(int, int);
int NumRanksSide();
double xGrid();
double yGrid();
double xMin();
double xMax();
double yMin();
double yMax();
int xQuad();
int yQuad();
////////////////////////////////////////////////////////////////////////////////

#endif
