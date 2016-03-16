#include "simulator.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>

// Only basic primatives of the simulation are made global variables.
// Everything else derived from these are made into inline functions
static Boid* boids;
static int myrank;
static int mynumboids;
static int numranks;
static double sidelen;
static double cutoff = 1.0;

void InitializeSim(Boid* b, int mr, int mnb, int nr, double sl)
{
    boids = b;
    myrank = mr;
    mynumboids = mnb;
    numranks = nr;
    sidelen = sl;
}

void Iterate(int ticknum)
{
    int* neighbor_ranks = NULL;
    Boid* neighbor_boids = NULL;
    int* num_neighbor_boids = NULL;
    int num_neighbors = -1;

    Neighbors(&neighbor_ranks, &num_neighbors);

    num_neighbor_boids = SendRecvNumBoids(neighbor_ranks, num_neighbors,
                                          ticknum);
    neighbor_boids = SendRecvBoids(neighbor_ranks, num_neighbor_boids,
                                   num_neighbors, ticknum);

}

Boid* SendRecvBoids(int* neighbor_ranks, int* num_neighbor_boids,
                    int num_neighbors, int ticknum)
{
    int i, j, rank, idx = 0, total_neighbors = 0;
    for (i = 0; i < num_neighbors; ++i)
        total_neighbors += num_neighbor_boids[i];

    Boid* neighbor_boids = (Boid*) malloc(total_neighbors * sizeof(Boid));
    Boid** temp_boids = (Boid**) malloc(num_neighbors * sizeof(Boid*));
    MPI_Request*
    send_r = (MPI_Request*) malloc(num_neighbors * sizeof(MPI_Request));
    MPI_Request*
    recv_r = (MPI_Request*) malloc(num_neighbors * sizeof(MPI_Request));

    for (i = 0; i < num_neighbors; ++i) {
        rank = neighbor_ranks[i];
        temp_boids[i] = (Boid*) malloc( num_neighbor_boids[i] * sizeof(Boid));
        MPI_Isend(boids, 4 * mynumboids, MPI_DOUBLE, rank, ticknum,
                  MPI_COMM_WORLD, &send_r[i]);
        MPI_Irecv(temp_boids[i], 4 * num_neighbor_boids[i], MPI_DOUBLE, rank,
                  ticknum, MPI_COMM_WORLD, &recv_r[i]);
    }

    MPI_Waitall(num_neighbors, send_r, MPI_STATUSES_IGNORE);
    MPI_Waitall(num_neighbors, recv_r, MPI_STATUSES_IGNORE);

    // Linearize boids for easy running later
    for (i = 0; i < num_neighbors; ++i) {
        for (j = 0; j < num_neighbor_boids[i]; ++j) {
            neighbor_boids[idx++] = temp_boids[i][j];
        }
    }

    return neighbor_boids;
}

int* SendRecvNumBoids(int* neighbor_ranks, int num_neighbors, int ticknum)
{
    int* num_neighbor_boids = (int*) malloc(num_neighbors * sizeof(int));
    MPI_Request*
    send_r = (MPI_Request*) malloc(num_neighbors * sizeof(MPI_Request));
    MPI_Request*
    recv_r = (MPI_Request*) malloc(num_neighbors * sizeof(MPI_Request));

    int i, rank;
    for (i = 0; i < num_neighbors; ++i) {
        rank = neighbor_ranks[i];
        MPI_Isend(&mynumboids, 1, MPI_INT, rank, ticknum, MPI_COMM_WORLD,
                  &send_r[i]);
        MPI_Irecv(&num_neighbor_boids[i], 1, MPI_INT, rank, ticknum,
                  MPI_COMM_WORLD, &recv_r[i]);
    }

    MPI_Waitall(num_neighbors, send_r, MPI_STATUSES_IGNORE);
    MPI_Waitall(num_neighbors, recv_r, MPI_STATUSES_IGNORE);

    return num_neighbor_boids;
}


void Neighbors(int** ranks, int* num_neighbors)
{
    int i, j, idx = 0;

    // If there are only 4 ranks, then everyone who isn't you is a neighbor
    if (numranks == 4) {
        *num_neighbors = 3;
        *ranks = (int*) malloc( (*num_neighbors) * sizeof(int) );

        for (i = 0; i < 4; ++i) {
            if (i != myrank)
                (*ranks)[idx++] = i;
        }
    }
    // Otherwise, you have 8 unique neighbors, since numranks = 4^n
    else {
        int xnew, ynew, newrank;
        int xquad = xQuad();
        int yquad = yQuad();
        int num_rank_side = NumRanksSide();
        *num_neighbors = 8;
        *ranks = (int*) malloc( (*num_neighbors) * sizeof(int) );

        for (i = -1; i <= 1; ++i) {
            for (j = -1; j <= 1; ++j) {
                if (i != 0 || j != 0) {
                    xnew = mod(xquad + i, num_rank_side);
                    ynew = mod(yquad + j, num_rank_side);
                    (*ranks)[idx++] = QuadToRank(xnew, ynew);
                }
            }
        }
    }
}

////////////////////////////////////////////////////////////////////////////////
// Got wrapping modular function from:
// http://stackoverflow.com/questions/4003232
// Standard C modulus does not wrap around
// a % b
int mod(int a, int b)
{
    if(b < 0)
        return mod(-a, -b);
    int ret = a % b;
    if(ret < 0)
        ret += b;
    return ret;
}
////////////////////////////////////////////////////////////////////////////////

inline double xGrid()
{
    return sidelen / sqrt(numranks);
}
inline double yGrid()
{
    return sidelen / sqrt(numranks);
}
inline int NumRanksSide()
{
    return numranks / (int)sqrt(numranks);
}
inline int xQuad()
{
    return myrank % NumRanksSide();
}
inline int yQuad()
{
    return myrank / NumRanksSide();
}
inline double xMin()
{
    return xQuad() * xGrid();
}
inline double xMax()
{
    return (xQuad() + 1) * xGrid();
}
inline double yMin()
{
    return yQuad() * yGrid();
}
inline double yMax()
{
    return (yQuad() + 1) * yGrid();
}
inline int QuadToRank(int x, int y)
{
    return x + NumRanksSide() * y;
}
