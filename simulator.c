
#include "simulator.h"
#include "clcg4.h"
#include "io.h"
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <mpi.h>
#include <assert.h>

/*
 * Global statics. Persistent between iteration calls. C version of having nice class variables
 * Only basic primatives of the simulation are made global variables.
 * Everything else derived from these are made into functions
 */
static Boid* boids;
static char* fname;
static int seed;
static int myrank;
static int numranks;
static int mynumboids;
static int global_numboids;
static double dt;
static double noise;
static double boid_v;
static double cutoff;
static double sidelen;

/*
 * Basic initialization of static variables based off Config struct, read in from ini file,
 * as well as MPI specific variables
 */
void
InitializeSim(Boid* b, Config* c, int mr, int mnb, int nr)
{
    boids = b;
    myrank = mr;
    numranks = nr;
    mynumboids = mnb;

    dt = c->dt;
    boid_v = c->v;
    seed = c->seed;
    fname = c->fname;
    noise = c->noise;
    cutoff = c->cutoff;
    sidelen = c->sidelen;
    global_numboids = c->numboids;
}

/*
 * Driver of the simulator. Called with ticknum so sending and receiving from other MPI ranks
 * can only be done among the same ticknum (used as MPI send/recv tag)
 */
void
Iterate(int ticknum)
{
    double avg_norm_v;
    Boid* all_boids = NULL;
    int* neighbor_ranks = NULL;
    Boid* neighbor_boids = NULL;
    int* num_neighbor_boids = NULL;
    int num_neighbors, neighbor_total;

    /* Find who rank numbers of neighbor ranks */
    Neighbors(&neighbor_ranks, &num_neighbors);

    /* Make sure each rank has the number of neighbor boids it's supposed to receive */
    num_neighbor_boids = SendRecvNumBoids(neighbor_ranks, num_neighbors, ticknum);

    /* Actually send boids */
    neighbor_boids = SendRecvBoids(neighbor_ranks, num_neighbor_boids, num_neighbors, ticknum);

    /* Finds the total number of neighbor boids from other ranks */
    neighbor_total = TotalNeighborBoids(num_neighbor_boids, num_neighbors);

    /* Smashes SendRecvBoids lists into one list for easy iteration */
    all_boids = ConcatenateBoids(neighbor_boids, neighbor_total);

    /* Write all data before changing. Uses MPI IO for parallelism */
    WriteRankData(fname, boids, mynumboids, global_numboids, ticknum, myrank, numranks);

    /* Update position and velocity */
    UpdateVelocity(all_boids, neighbor_total);
    UpdatePosition(neighbor_ranks, num_neighbors, ticknum);

    /* Calculates statistic used in Tamas's paper */
    avg_norm_v = AverageNormalizedVelocity();

    /* Makes sure no boids have been lost, and all boids are where they're supposed to be. In the
       interest of speed, this function should probably be commented out for production runs */
    SanityCheck();
}

/*
 * Calculates parameter dictating how ordered the boids are for the phase change behaviors.
 * See Tamas's paper for more details. Currently this function is called by doesn't actually output
 * anywhere
 */
double
AverageNormalizedVelocity(void)
{
    /* Each rank calculates its local part */
    int i;
    double vx = 0.0, vy = 0.0;
    double* vx_list = NULL;
    double* vy_list = NULL;
    for (i = 0; i < mynumboids; ++i) {
        vx += boids[i].v.x;
        vy += boids[i].v.y;
    }

    /* Only rank 0 does the summing. All other ranks just return 0 at the end */
    if (myrank == 0) {
        vx_list = (double*) calloc(numranks, sizeof(double));
        vy_list = (double*) calloc(numranks, sizeof(double));
    }

    MPI_Gather(&vx, 1, MPI_DOUBLE, vx_list, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);
    MPI_Gather(&vy, 1, MPI_DOUBLE, vy_list, 1, MPI_DOUBLE, 0, MPI_COMM_WORLD);

    if (myrank == 0) {
        vx = 0.0;
        vy = 0.0;

        for (i = 0; i < numranks; ++i) {
            vx += vx_list[i];
            vy += vy_list[i];
        }

        return sqrt(vx * vx + vy * vy) / (global_numboids * boid_v);
    }

    return 0.0;
}


/*
 * Runs through boids and updates their positions based off their velocities
 * Enforces global boundary conditions and makes sure that if a boid moves
 * out of territory controlled by its rank, it is moved to the appropriate rank
 */
void
UpdatePosition(int* neighbor_ranks, int num_neighbors, int ticknum)
{
    double newx, newy;
    int i, rank, idx;

    /* Initialize to 0, parallels neighbor_ranks array */
    int* num_to_send = (int*) calloc(num_neighbors, sizeof(int));
    int* index_cache = (int*) calloc(mynumboids, sizeof(int));

    for (i = 0; i < mynumboids; ++i) {
        newx = boids[i].r.x + boids[i].v.x * dt;
        newy = boids[i].r.y + boids[i].v.y * dt;

        /* Enforce global periodic boundary conditions */
        if (newx > sidelen) newx -= sidelen;
        if (newy > sidelen) newy -= sidelen;
        if (newx < 0.0) newx += sidelen;
        if (newy < 0.0) newy += sidelen;

        boids[i].r.x = newx;
        boids[i].r.y = newy;
        rank = CheckLocalBoundaries(newx, newy);

        /* Checks if current boid needs to be sent to a different rank */
        if (rank != myrank) {
            idx = IndexOf(neighbor_ranks, num_neighbors, rank);
            num_to_send[idx]++;
            index_cache[i] = idx;
        }
        else
            index_cache[i] = -1;
    }

    /* Sends out-of-place boids to required ranks */
    RearrangeBoids(neighbor_ranks, num_to_send, index_cache, num_neighbors, ticknum);

    free(num_to_send);
    free(index_cache);
}


/*
 * Takes the information generated in UpdatePosition, and moves out-of-place
 * boids to different ranks
 */
void
RearrangeBoids(int* neighbor_ranks, int* num_send, int* index_cache, int num_neighbors, int ticknum)
{
    int i, j, idx, rank;
    int total_sent = 0;
    int total_recv = 0;
    int* num_recv = (int*) calloc(num_neighbors, sizeof(int));
    Boid** boid_send = (Boid**) calloc(num_neighbors, sizeof(Boid*));
    Boid** boid_recv = (Boid**) calloc(num_neighbors, sizeof(Boid*));

    MPI_Request* send_r = (MPI_Request*) calloc(num_neighbors, sizeof(MPI_Request));
    MPI_Request* recv_r = (MPI_Request*) calloc(num_neighbors, sizeof(MPI_Request));

    // Prepares boids that need to be sent, and send the number of boids
    // to recv'd on the other end
    for (i = 0; i < num_neighbors; ++i) {
        idx = 0;
        rank = neighbor_ranks[i];

        MPI_Isend(&num_send[i], 1, MPI_INT, rank, ticknum, MPI_COMM_WORLD, &send_r[i]);
        MPI_Irecv(&num_recv[i], 1, MPI_INT, rank, ticknum, MPI_COMM_WORLD, &recv_r[i]);

        if (num_send[i] > 0) {
            total_sent += num_send[i];
            boid_send[i] = (Boid*) calloc(num_send[i], sizeof(Boid));
            for (j = 0; j < mynumboids; ++j) {
                if (index_cache[j] == i)
                    boid_send[i][idx++] = boids[j];
            }
        }
    }

    MPI_Waitall(num_neighbors, send_r, MPI_STATUSES_IGNORE);
    MPI_Waitall(num_neighbors, recv_r, MPI_STATUSES_IGNORE);

    // Sends actual boids
    for (i = 0; i < num_neighbors; ++i) {
        rank = neighbor_ranks[i];
        if (num_send[i] > 0) {
            MPI_Isend(boid_send[i], num_send[i] * sizeof(Boid), MPI_BYTE, rank, ticknum,
                      MPI_COMM_WORLD, &send_r[i]);
        }

        if (num_recv[i] > 0) {
            total_recv += num_recv[i];
            boid_recv[i] = (Boid*) calloc(num_recv[i], sizeof(Boid));
            MPI_Irecv(boid_recv[i], num_recv[i] * sizeof(Boid), MPI_BYTE, rank, ticknum,
                      MPI_COMM_WORLD, &recv_r[i]);
        }

    }

    MPI_Waitall(num_neighbors, send_r, MPI_STATUSES_IGNORE);
    MPI_Waitall(num_neighbors, recv_r, MPI_STATUSES_IGNORE);

    // After receiving or getting rid of boids, recombines them into an
    // intelligible form
    RecombineBoids(boid_recv, num_recv, index_cache, num_neighbors, total_sent, total_recv);

    for (i = 0; i < num_neighbors; ++i) {
        if (num_recv[i] > 0)
            free(boid_recv[i]);
        if (num_send[i] > 0)
            free(boid_send[i]);
    }

    free(boid_recv);
    free(boid_send);
    free(num_recv);
    free(send_r);
    free(recv_r);
}




// "Flattens" boids, and makes sure mynumboids matches the number of boids
// associated with this rank
void RecombineBoids(Boid** boid_recv, int* num_recv, int* index_cache, int num_neighbors,
                    int total_sent, int total_recv)
{
    int i, j;
    int idx = 0;
    int new_total = mynumboids + total_recv - total_sent;
    Boid* new_boids;

    new_boids = (Boid*) calloc(new_total, sizeof(Boid));

    for (i = 0; i < mynumboids; ++i) {
        if (index_cache[i] == -1) {
            new_boids[idx++] = boids[i];
        }
    }

    for (i = 0; i < num_neighbors; ++i) {
        if (num_recv[i] > 0) {
            for (j = 0; j < num_recv[i]; ++j) {
                new_boids[idx++] = boid_recv[i][j];
            }
        }
    }

    free(boids);
    boids = new_boids;
    mynumboids = new_total;
}




// Checks to make sure that all boids are in the proper spaces, and that no
// boids have been lost globally. Crashes program if any checks are not met
void SanityCheck()
{
    int i, total_boids;
    double xmin = xMin();
    double ymin = yMin();
    double xmax = xMax();
    double ymax = yMax();
    Boid b;

    // Sums up the number of boids on each rank
    MPI_Allreduce(&mynumboids, &total_boids, 1, MPI_INT, MPI_SUM, MPI_COMM_WORLD);

    assert(total_boids == global_numboids);

    for (i = 0; i < mynumboids; ++i) {
        b = boids[i];

        assert(sqrt(b.v.x * b.v.x + b.v.y * b.v.y) <= boid_v * 1.01);
        assert(b.r.x < xmax);
        assert(b.r.x > xmin);
        assert(b.r.y < ymax);
        assert(b.r.y > ymin);
    }
}




// I know this is inefficient, but I'm not creating a binary search tree or
// hash table just to search through, at maximum, 8 values
int IndexOf(int* neighbor_ranks, int num_neighbors, int rank)
{
    int i;
    for (i = 0; i < num_neighbors; ++i) {
        if (neighbor_ranks[i] == rank)
            return i;
    }
    fprintf(stderr, "Boid moved outside 8 neighbors");
    exit(1);
}




// Checks the rank the position (x, y) belongs to
int CheckLocalBoundaries(double x, double y)
{
    int xquad = (int) floor(x / xGrid());
    int yquad = (int) floor(y / yGrid());
    return QuadToRank(xquad, yquad);
}




// Updates all boids for this simulator based of all_boids, which include boids
// in the neighboring 8 ranks (if numranks > 4)
void UpdateVelocity(Boid* all_boids, int neighbor_total)
{
    int i, j, neighbors, total_count = neighbor_total + mynumboids;
    double v_x, v_y, angle;
    Vec v;
    srand( time(NULL) );
    int seed = rand() % Maxgen;

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
        angle = VecAngle(v) + noise * (GenVal(seed) - 0.5);
        VecSetAngle(&v, angle);
        VecSetLength(&v, boid_v);

        boids[i].v = v;
    }

    free(all_boids);
}




// Concatenates boids from simulator with boids found in 8 neighbor ranks, and
// returns them
Boid* ConcatenateBoids(Boid* neighbor_boids, int neighbor_total)
{
    int total_count = neighbor_total + mynumboids;
    Boid* all_boids = (Boid*) calloc(total_count, sizeof(Boid));
    int i, idx = 0;
    for (i = 0; i < mynumboids; ++i)
        all_boids[idx++] = boids[i];

    for (i = 0; i < neighbor_total; ++i)
        all_boids[idx++] = neighbor_boids[i];

    free(neighbor_boids);

    return all_boids;
}




// Counts the total number of neighbors in neighboring 8 ranks  based of
// num_neighbor_boids array
int TotalNeighborBoids(int* num_neighbor_boids, int num_neighbors)
{
    int i, neighbor_total = 0;
    for (i = 0; i < num_neighbors; ++i)
        neighbor_total += num_neighbor_boids[i];

    return neighbor_total;
}




// Sends and receives boids from neighboring 8 ranks
Boid* SendRecvBoids(int* neighbor_ranks, int* num_neighbor_boids,
                    int num_neighbors, int ticknum)
{

    int i, j, rank, idx = 0;
    int neighbor_total = TotalNeighborBoids(num_neighbor_boids, num_neighbors);

    Boid* neighbor_boids = (Boid*) calloc(neighbor_total, sizeof(Boid));
    Boid** temp_boids = (Boid**) calloc(num_neighbors, sizeof(Boid*));

    MPI_Request* send_r = (MPI_Request*) calloc(num_neighbors, sizeof(MPI_Request));
    MPI_Request* recv_r = (MPI_Request*) calloc(num_neighbors, sizeof(MPI_Request));

    for (i = 0; i < num_neighbors; ++i) {
        rank = neighbor_ranks[i];
        temp_boids[i] = (Boid*) calloc( num_neighbor_boids[i], sizeof(Boid));
        MPI_Isend(boids, mynumboids * sizeof(Boid), MPI_BYTE, rank, ticknum, MPI_COMM_WORLD, &send_r[i]);
        MPI_Irecv(temp_boids[i], num_neighbor_boids[i] * sizeof(Boid), MPI_BYTE, rank, ticknum,
                  MPI_COMM_WORLD, &recv_r[i]);
    }

    MPI_Waitall(num_neighbors, send_r, MPI_STATUSES_IGNORE);
    MPI_Waitall(num_neighbors, recv_r, MPI_STATUSES_IGNORE);

    // Linearize boids for easy running later
    for (i = 0; i < num_neighbors; ++i) {
        for (j = 0; j < num_neighbor_boids[i]; ++j) {
            neighbor_boids[idx++] = temp_boids[i][j];
        }
        free(temp_boids[i]);
    }

    free(send_r);
    free(recv_r);
    free(temp_boids);

    return neighbor_boids;
}




// Sends and receives the number of boids, so MPI knows how much to receive
// in a later call
int* SendRecvNumBoids(int* neighbor_ranks, int num_neighbors, int ticknum)
{
    int* num_neighbor_boids = (int*) calloc(num_neighbors, sizeof(int));
    MPI_Request* send_r = (MPI_Request*) calloc(num_neighbors, sizeof(MPI_Request));
    MPI_Request* recv_r = (MPI_Request*) calloc(num_neighbors, sizeof(MPI_Request));

    int i, rank;
    for (i = 0; i < num_neighbors; ++i) {
        rank = neighbor_ranks[i];
        MPI_Isend(&mynumboids, 1, MPI_INT, rank, ticknum, MPI_COMM_WORLD, &send_r[i]);
        MPI_Irecv(&num_neighbor_boids[i], 1, MPI_INT, rank, ticknum, MPI_COMM_WORLD, &recv_r[i]);
    }

    MPI_Waitall(num_neighbors, send_r, MPI_STATUSES_IGNORE);
    MPI_Waitall(num_neighbors, recv_r, MPI_STATUSES_IGNORE);

    free(send_r);
    free(recv_r);

    return num_neighbor_boids;
}




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
        *ranks = (int*) calloc( (*num_neighbors), sizeof(int) );

        for (i = 0; i < 4; ++i) {
            if (i != myrank)
                (*ranks)[idx++] = i;
        }
    }
    // Otherwise, you have 8 unique neighbors, since numranks = 4^n
    else {
        int xnew, ynew;
        int xquad = xQuad();
        int yquad = yQuad();
        int num_rank_side = NumRanksSide();
        *num_neighbors = 8;
        *ranks = (int*) calloc( (*num_neighbors), sizeof(int) );

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
