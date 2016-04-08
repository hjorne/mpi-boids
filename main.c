/* PFlockC Application
   Written by Joe Horne */

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <mpi.h>
#include "simulator.h"
#include "clcg4.h"
#include "boid.h"
#include "init.h"
#include "io.h"

int
main(int argc, char** argv)
{
    int myrank, numranks, mynumboids, i;
    double starttime = 0.0;
    Boid* boids = NULL;
    Config* c = NULL;

    /* MPI and clcg4 initialization */
    MPI_Init( &argc, &argv);
    MPI_Comm_size( MPI_COMM_WORLD, &numranks);
    MPI_Comm_rank( MPI_COMM_WORLD, &myrank);
    InitDefault();

    if (myrank == 0)
        starttime = MPI_Wtime();

    c = ReadConfig(argv[1]);

    /* Initialize boids and simulator */
    Initialize(&boids, c, &mynumboids, myrank, numranks);
    InitializeSim(boids, c, myrank,  mynumboids, numranks);

    /* Make sure everybody is initialized before beginning iteration */
    MPI_Barrier( MPI_COMM_WORLD );
    for (i = 0; i < c->numticks; ++i)
        Iterate(i);
    /* Make sure everybody finishes iterating before completing sim */
    MPI_Barrier( MPI_COMM_WORLD );

    if (myrank == 0)
        printf("That took %f seconds\n", MPI_Wtime() - starttime);

    MPI_Finalize();
    return 0;
}
