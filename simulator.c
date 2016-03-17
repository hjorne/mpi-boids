////////////////////////////////////////////////////////////////////////////////
#include "simulator.h"
#include "clcg4.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Global statics. Persistent between iteration calls
// Only basic primatives of the simulation are made global variables.
// Everything else derived from these are made into functions
static Boid* boids;
static int myrank;
static int mynumboids;
static int numranks;
static double sidelen;
static double cutoff = 1.0;
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Basic initialization of static variables
void InitializeSim(Boid* b, int mr, int mnb, int nr, double sl)
{
    boids = b;
    myrank = mr;
    mynumboids = mnb;
    numranks = nr;
    sidelen = sl;
}
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Driver of the simulator. Called with ticknum so sending and receiving from
// other MPI ranks can only be done among the same ticknum (used as tag)
void Iterate(int ticknum)
{
    int* neighbor_ranks = NULL;
    Boid* neighbor_boids = NULL;
    Boid* all_boids = NULL;
    int* num_neighbor_boids = NULL;
    int num_neighbors, neighbor_total;

    Neighbors(&neighbor_ranks, &num_neighbors);

    num_neighbor_boids = SendRecvNumBoids(neighbor_ranks, num_neighbors,
                                          ticknum);
    neighbor_boids = SendRecvBoids(neighbor_ranks, num_neighbor_boids,
                                   num_neighbors, ticknum);
    neighbor_total = TotalNeighborBoids(num_neighbor_boids, num_neighbors);
    all_boids = ConcatenateBoids(neighbor_boids, neighbor_total);

    UpdateBoids(all_boids, neighbor_total);
    // TODO: Still need to move boids based on velocity, then among ranks
}
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Updates all boids for this simulator based of all_boids, which include boids
// in the neighboring 8 ranks (if numranks > 4)
void UpdateBoids(Boid* all_boids, int neighbor_total)
{
    int i, j, neighbors, total_count = neighbor_total + mynumboids;
    double v_x, v_y, angle;
    Vec v;

    for (i = 0; i < mynumboids; ++i) {
        neighbors = 0;
        v_x = 0.0;
        v_y = 0.0;
        for (j = 0; j < total_count; ++j) {
            if ( BoidDist(boids[i], all_boids[j]) < cutoff ) {
                ++neighbors;
                v_x += all_boids[j].v.x;
                v_y += all_boids[j].v.y;
            }
        }
        v.x = v_x / (double) neighbors;
        v.y = v_y / (double) neighbors;
        angle = VecAngle(v) + (GenVal(1) - 0.5);
        VecSetAngle(&v, angle);
        VecSetLength(&v, 0.03);

        boids[i].v = v;
    }
}
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Concatenates boids from simulator with boids found in 8 neighbor ranks, and
// returns them
Boid* ConcatenateBoids(Boid* neighbor_boids, int neighbor_total)
{
    int total_count = neighbor_total + mynumboids;
    Boid* all_boids = (Boid*) malloc(total_count * sizeof(Boid));
    int i, idx = 0;
    for (i = 0; i < mynumboids; ++i)
        all_boids[idx++] = boids[i];

    for (i = 0; i < neighbor_total; ++i)
        all_boids[idx++] = neighbor_boids[i];

    return all_boids;
}
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Counts the total number of neighbors in neighboring 8 ranks  based of
// num_neighbor_boids array
int TotalNeighborBoids(int* num_neighbor_boids, int num_neighbors)
{
    int i, neighbor_total = 0;
    for (i = 0; i < num_neighbors; ++i)
        neighbor_total += num_neighbor_boids[i];
    return neighbor_total;
}
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Sends and receives boids from neighboring 8 ranks
Boid* SendRecvBoids(int* neighbor_ranks, int* num_neighbor_boids,
                    int num_neighbors, int ticknum)
{

    int i, j, rank, idx = 0;
    int neighbor_total = TotalNeighborBoids(num_neighbor_boids, num_neighbors);

    Boid* neighbor_boids = (Boid*) malloc(neighbor_total * sizeof(Boid));
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
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Sends and receives the number of boids, so MPI knows how much to receive
// in a later call
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
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Finds exactly which ranks are neighboring ranks, and how many neighboring
// ranks you have
//
// Since numranks is always a power of 4, if numranks == 4, then you have 3
// neighbors, and Otherwise you always have 8. The general assumption when
// discussing the code above is that you have 8 neighbors
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


////////////////////////////////////////////////////////////////////////////////
// Got wrapping modular function from Armen Tsirunyan on Stack Overflow
// http://stackoverflow.com/a/4003287/3407785
// Standard C modulus does not wrap around
// (a % b)
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


////////////////////////////////////////////////////////////////////////////////
// A bunch of functions that I would inline of IBM's XL compiler would let me
double xGrid()
{
    return sidelen / sqrt(numranks);
}
double yGrid()
{
    return sidelen / sqrt(numranks);
}
int NumRanksSide()
{
    return numranks / (int)sqrt(numranks);
}
int xQuad()
{
    return myrank % NumRanksSide();
}
int yQuad()
{
    return myrank / NumRanksSide();
}
double xMin()
{
    return xQuad() * xGrid();
}
double xMax()
{
    return (xQuad() + 1) * xGrid();
}
double yMin()
{
    return yQuad() * yGrid();
}
double yMax()
{
    return (yQuad() + 1) * yGrid();
}
int QuadToRank(int x, int y)
{
    return x + NumRanksSide() * y;
}
////////////////////////////////////////////////////////////////////////////////
