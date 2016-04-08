#ifndef _BOID_H_
#define _BOID_H_

#include "vec.h"

/* r is the position vector,
   v is the velocity vector, 
   id is a unique number between two boids */
typedef struct boid_s {
    Vec r;
    Vec v;
    unsigned int id;
} Boid;

double BoidDist(Boid b1, Boid b2);

#endif
