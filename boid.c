#include "boid.h"
#include <math.h>

double BoidDist(Boid b1, Boid b2)
{
    double dx = b2.r.x - b1.r.x;
    double dy = b2.r.y - b1.r.y;
    return sqrt(dx*dx + dy*dy);
}
