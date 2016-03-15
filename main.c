////////////////////////////////////////////////////////////////////////////////
// PFlockC Application                                                        //
// Written by Joe Horne                                                       //
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <mpi.h>
#include "simulator.h"
#include "boid.h"
#include "init.h"
////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////
int main(int argc, char** argv)
{
    int myrank, numranks, mynumboids, numboids;
    double starttime, sidelen;
    Boid* boids = NULL;

    MPI_Init( &argc, &argv);
    MPI_Comm_size( MPI_COMM_WORLD, &numranks);
    MPI_Comm_rank( MPI_COMM_WORLD, &myrank);
    InitDefault();  // clcg4 initialization

    if (myrank == 0)
        starttime = MPI_Wtime();

    numboids = atoi(argv[1]);
    sidelen = atof(argv[2]);

    Initialize(&boids, &mynumboids, myrank, numranks, numboids, sidelen);

    MPI_Barrier( MPI_COMM_WORLD );

    if (myrank == 0)
        printf("That took %f seconds\n", MPI_Wtime() - starttime);

    MPI_Finalize();

    return 0;
}
////////////////////////////////////////////////////////////////////////////////
