#ifndef _SIMULATOR_H_
#define _SIMULATOR_H_

#include "boid.h"

void InitializeSim(Boid*, int, int, int, double);
void Iterate(int);
void Neighbors(int**, int*);
int* SendRecvNumBoids(int*, int, int);
Boid* SendRecvBoids(int*, int*, int, int);

int mod(int, int);

inline double xGrid();
inline double yGrid();
inline int xQuad();
inline int yQuad();
inline double xMin();
inline double xMax();
inline double yMin();
inline double yMax();
inline int QuadToRank(int, int);

#endif
