#include "simulator.h"
#include <math.h>
#include <time.h>

Simulator::Simulator(int myrank_, int numranks_, int numboids_, double sidelen,
                     double cutoff_radius_ = 5.0)
{
    myrank = myrank_;
    numranks = numranks_;
    numboids = numboids_ / numranks; // numboids is numboids per rank

    x_width = sidelen;
    y_width = sidelen;

    x_grid_width = x_width / sqrt((double) numranks);
    y_grid_width = y_width / sqrt((double) numranks);

    cutoff_radius = cutoff_radius_;

    // Define integer quadrants based on ranks
    int x_quad = myrank % numranks;
    int y_quad = myrank / numranks;

    xmin = x_quad * x_grid_width;
    xmax = (x_quad + 1) * x_grid_width;
    ymin = y_quad * y_grid_width;
    ymax = (y_quad + 1) * y_grid_width;

    double x, y;
    srand( time(NULL) );
    gen.init( rand() % 100000 );
    for (int i = 0; i < numboids; ++i) {
        x = gen.random() * x_grid_width + xmin;
        y = gen.random() * y_grid_width + ymin;

    }
}

int Simulator::PosToRank(Vec const &v)
{

}

Vec &RankToPos(int i)
{

}
