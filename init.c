////////////////////////////////////////////////////////////////////////////////
#include "clcg4.h"
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <mpi.h>
#include "init.h"
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
void Initialize(Boid** boids, int* mynumboids, int myrank, int numranks,
                int numboids, double sidelen)
{
    CheckRanks(myrank, numranks);

    if (myrank == 0) {
        InitializeRanks(boids, mynumboids, numranks, numboids, sidelen);
    }
    else {
        MPI_Recv(mynumboids, 1, MPI_INT, 0, 0, MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);

        *boids = (Boid*) malloc( (*mynumboids) * sizeof(Boid) );
        MPI_Recv(*boids, 4 * (*mynumboids), MPI_DOUBLE, 0, 1, MPI_COMM_WORLD,
                 MPI_STATUS_IGNORE);
    }
}
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
void InitializeRanks(Boid** myboids, int* mynumboids, int numranks,
                     int numboids, double sidelen)
{
    Vec* boid_positions = InitBoidPositions(numranks, numboids, sidelen);
    int* boid_ranks = BoidRanks(boid_positions, numranks, numboids, sidelen);
    int* boids_per_rank = DistributeBoids(boid_positions, numboids, numranks,
                                          sidelen);

    Boid* boids = NULL;
    int rank, j, idx;
    // Send boids to all nonzero ranks
    for (rank = 1; rank < numranks; ++rank) {
        boids = (Boid*) malloc( boids_per_rank[rank] * sizeof(Boid) );

        // Send number of boids this rank will be receiving
        MPI_Send(&boids_per_rank[rank], 1, MPI_INT, rank, 0, MPI_COMM_WORLD);
        idx = 0;
        for (j = 0; j < numboids; ++j) {
            if ( boid_ranks[j] == rank ) {
                boids[idx].r = boid_positions[j];
                VecRandomAngle(&(boids[idx++].v), 0.03);
            }
        }
        MPI_Send(boids, 4 * boids_per_rank[rank], MPI_DOUBLE, rank, 1,
                 MPI_COMM_WORLD);
    }

    // Handle rank 0
    idx = 0;
    *myboids = (Boid*) malloc( boids_per_rank[0] * sizeof(Boid) );
    for (j = 0; j < numboids; ++j) {
        if (boid_ranks[j] == 0) {
            (*myboids)[idx].r = boid_positions[j];
            VecRandomAngle( &((*myboids)[idx++].v), 0.03 );
        }
    }
    *mynumboids = boids_per_rank[0];
}
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// void InitializeRank0()
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// For the ith boid in boid_positions, create a `parallel` array where the ith
// value is the rank the ith boid is going to be
int* BoidRanks(Vec* boid_positions, int numranks, int numboids, double sidelen)
{
    int* boid_ranks = (int*) malloc( numboids * sizeof(int) );

    int i;
    for (i = 0; i < numboids; ++i)
        boid_ranks[i] = VecToRank(boid_positions[i], sidelen, numranks);
    return boid_ranks;
}
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Based off location specified in Vec v, as well as the length along the total
// side of the simulation and the number of ranks, this function returns the
// rank where the boid belongs
int VecToRank(Vec v, double sidelen, int numranks)
{
    double x_width = sidelen / sqrt((double) numranks);
    double y_width = sidelen / sqrt((double) numranks);

    int x_quad = (int) floor(v.x / x_width);
    int y_quad = (int) floor(v.y / y_width);

    int ranks_per_side = (int) floor(sqrt((double) numranks));

    return x_quad + y_quad * ranks_per_side;
}
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// This information is necessary so each rank knows how many boids it's
// supposed to receive for MPI_recv.
//
// Information about which position goes where could be saved for later so it
// doesn't have to be recalculated, but that would add complications to what
// is a quick O(N) calculation, so it's not saved
int* DistributeBoids(Vec* boid_positions, int numboids, int numranks,
                     double sidelen)
{
    // Calloc initilizes all values to 0
    int* boids_per_rank = (int*) calloc( numranks, sizeof(int) );
    int* boid_ranks = BoidRanks(boid_positions, numranks, numboids, sidelen);
    int i, rank;

    for (i = 0; i < numboids; ++i)
        boids_per_rank[boid_ranks[i]]++;

    return boids_per_rank;
}
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Initializes all boids. This needs to be done on a single rank, so a
// realistic distribution can be obtained. Since each rank will have
// its own Simulator object, this is done early in the main function instead
Vec* InitBoidPositions(int numranks, int numboids, double sidelen)
{
    Vec* boid_positions = (Vec*) malloc( numboids * sizeof(Vec) );

    srand( time(NULL) );
    int seed = rand() % 16383;  // Maximum seed value specified in clcg4.h

    seed = 1;
    int i;
    for (i = 0; i < numboids; ++i) {
        boid_positions[i].x = GenVal(seed) * sidelen;
        boid_positions[i].y = GenVal(seed) * sidelen;
    }
    return boid_positions;
}
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Checks that numranks is a power of 4, as is assumed by the program. Prints
// out error if rank 0
void CheckRanks(int myrank, int numranks)
{
    double splitlvl = log( (double)numranks ) / log(4);
    if (floor(splitlvl) != splitlvl) {
        if (myrank == 0)
            fprintf(stderr, "Number of ranks needs to be a power of 4\n");
        exit(1);
    }
}
////////////////////////////////////////////////////////////////////////////////
