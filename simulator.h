#ifndef _SIMULATOR_H_
#define _SIMULATOR_H_

////////////////////////////////////////////////////////////////////////////////
#include "boid.h"
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
void InitializeSim(Boid*, int, int, int, int, double);
Boid* SendRecvBoids(int*, int*, int, int);
int* SendRecvNumBoids(int*, int, int);
Boid* ConcatenateBoids(Boid*, int);
int TotalNeighborBoids(int*, int);
void UpdateVelocity(Boid*, int);
void UpdatePosition(int*, int, int);
void Neighbors(int**, int*);
int mod(int, int);
void Iterate(int);
int CheckLocalBoundaries(double, double);
int IndexOf(int*, int, int);

void RearrangeBoids(int*, int*, int*, int, int);
void RecombineBoids(Boid**, int*, int*, int, int, int);
void SanityCheck();
void PrintBoids();
void PrintBoid(Boid);
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
