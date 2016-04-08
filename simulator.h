#ifndef _SIMULATOR_H_
#define _SIMULATOR_H_

#include "boid.h"
#include "io.h"

/* Iterates through timestep passed into function */
void Iterate(int);

/* Wrapping modulus function */
int mod(int, int);

/* Checks to make sure simulation is running bug-free */
void SanityCheck(void);

/* Finds and returns the index the rank in a list of neighboring ranks */
int IndexOf(int*, int, int);

/* Finds who the neighbors of a rank are */
void Neighbors(int**, int*);

/* Goes through all the boids and updates the velocities */
void UpdateVelocity(Boid*, int);

/* Finds the total number of neighboring boids */
int TotalNeighborBoids(int*, int);

/* Calculates Tamas's statistic */
double AverageNormalizedVelocity(void);

/* Concatenates all neighbor boids for easy iteration */
Boid* ConcatenateBoids(Boid*, int);

/* Updates positions of all boids in accordance with velocity */
void UpdatePosition(int*, int, int);

/* Sends and receives how many boids each rank should expect */
int* SendRecvNumBoids(int*, int, int);

/* Checks that a position is within proper boundarys of the rank */
int CheckLocalBoundaries(double, double);

/* Sends and receives actual neighbor boids */
Boid* SendRecvBoids(int*, int*, int, int);

/* If a boid is outside of its proper rank space, move it to the right rank */
void RearrangeBoids(int*, int*, int*, int, int);

/* Initializes simulation static variables */
void InitializeSim(Boid*, Config*, int, int, int);

/* If a rank has received new boids from a neighbor rank, put these new boids into the boid array */
void RecombineBoids(Boid**, int*, int*, int, int, int);

/* Trivial functions that should be inlined, but IBM's XL compiler won't let me */
int xQuad(void);
int yQuad(void);
double xMin(void);
double xMax(void);
double yMin(void);
double yMax(void);
double xGrid(void);
double yGrid(void);
int NumRanksSide(void);
int QuadToRank(int, int);

#endif
