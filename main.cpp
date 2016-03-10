////////////////////////////////////////////////////////////////////////////////
// PFlock Application                                                         //
// Written by Joe Horne                                                       //
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
#include "random_generator.h"
#include "simulator.h"
#include "boid.h"
#include "vec.h"

#include <stdlib.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <mpi.h>
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
// Function declarations. Minimal, since essentially all actions take place
// in Simulator objects
std::vector< std::vector<Vec*> > InitBoidPositions(int, int, double);
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

    if (myrank == 0) {
        starttime = MPI_Wtime();

    }
    else {

    }

    numboids = 30;
    sidelen = 10;

    std::vector< std::vector<Vec*> >
        boid_positions = InitBoidPositions(numranks, numboids, sidelen);
    printf("Number of things %i\n", boid_positions[myrank].size());

    // Simulator rank_instance(myrank, numranks, numboids, sidelen, 5.0);

    MPI_Barrier( MPI_COMM_WORLD );


    MPI_Barrier( MPI_COMM_WORLD );

    if (myrank == 0)
        printf("That took %f seconds\n", MPI_Wtime() - starttime);
}
////////////////////////////////////////////////////////////////////////////////


////////////////////////////////////////////////////////////////////////////////
// Initializes all boids. This needs to be done on a single rank, so a
// realistic distribution can be obtained. Since each rank will have
// its own Simulator object, this is done early in the main function instead
std::vector< std::vector<Vec*> >
    InitBoidPositions(int numranks, int numboids, double sidelen)
{
    std::vector< std::vector<Vec*> > boid_positions(numranks);
    double x_width = sidelen / sqrt((double) numranks);
    double y_width = sidelen / sqrt((double) numranks);

    int x_quad, y_quad, rank, ranks_per_side;
    double x, y;
    srand( time(NULL) );
    RandomGenerator g( rand() % 100000 );

    ranks_per_side = (int) floor(sqrt((double) numranks));

    for (int i = 0; i < numboids; ++i) {
        x = g.random() * sidelen;
        y = g.random() * sidelen;

        x_quad = (int) floor(x / x_width);
        y_quad = (int) floor(y / y_width);

        rank = x_quad + y_quad * ranks_per_side;
        boid_positions[rank].push_back(new Vec(x, y));
    }
    return boid_positions;
}
////////////////////////////////////////////////////////////////////////////////
