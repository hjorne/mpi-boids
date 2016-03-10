#ifndef __SIMULATOR_H__
#define __SIMULATOR_H__

#include "vec.h"
#include "random_generator.h"

class Simulator {
public:
    Simulator(int, int, int, double, double);
    int PosToRank(Vec const&);
    Vec& RankToPos(int);

private:
    // numboids is the number of boids per rank
    int myrank, numranks, numboids;
    double x_width, y_width;
    double x_grid_width, y_grid_width;
    double xmin, xmax, ymin, ymax;

    double cutoff_radius;

    RandomGenerator gen;
};

#endif
