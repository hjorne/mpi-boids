////////////////////////////////////////////////////////////////////////////////
// PFlockC Application                                                        //
// Written by Joe Horne                                                       //
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <mpi.h>
#include "simulator.h"
#include "boid.h"
#include "vec.h"
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function declarations
Vec* InitBoidPositions(int, int, double);
int* DistributeBoids(Vec*, int, int);
int VecToRank(Vec, double, int);
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
    int myrank, numranks, numboids;
    double splitlvl, starttime, sidelen;

    MPI_Init( &argc, &argv);
    MPI_Comm_size( MPI_COMM_WORLD, &numranks);
    MPI_Comm_rank( MPI_COMM_WORLD, &myrank);

    // Kill all ranks if numranks is not a power of 4
    splitlvl = log(numranks) / log(4);
    if (floor(splitlvl) != splitlvl) {
        if (myrank == 0)
            fprintf(stderr, "Number of ranks needs to be a power of 4\n");
        return 1;
    }

    numboids = 30;
    sidelen = 10;

    if (myrank == 0) {
        starttime = MPI_Wtime();

    }
    else {

    }

    MPI_Barrier( MPI_COMM_WORLD );


    MPI_Barrier( MPI_COMM_WORLD );

    if (myrank == 0)
        printf("That took %f seconds\n", MPI_Wtime() - starttime);
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
int* DistributeBoids(Vec* boid_positions, int numboids, int numranks)
{
    // Calloc initilizes all values to 0
    int* boids_per_rank = (int*) calloc( numranks, sizeof(int) );

    int i, rank;
    for (i = 0; i < numboids; ++i) {
        rank = VecToRank(boid_positions[i]);
        boids_per_rank[rank]++;
    }
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
    int seed = rand() % 100000;

    int i;
    for (i = 0; i < numboids; ++i) {
        boid_positions[i].x = GenVal(seed) * sidelen;
        boid_positions[i].y = GenVal(seed) * sidelen;
    }
    return boid_positions;
}
////////////////////////////////////////////////////////////////////////////////
