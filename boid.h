#ifndef _BOID_H_
#define _BOID_H_

#include "vec.h"

typedef struct boid_s {
    Vec r;  // Position vector
    Vec v;  // Velocity vector
} Boid;

double BoidDist(Boid b1, Boid b2);

#endif
