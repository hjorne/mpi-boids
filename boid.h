#ifndef __BOID_H__
#define __BOID_H__

#include "vec.h"
#include <vector>

class Boid {
public:
    Boid() : r(Vec()), v(Vec()) {}; // Default constructor
    Boid(Vec const& r_, Vec const& v_) : r(r_), v(v_) {} // Explicit constructor
    Boid(Boid const& b) : r(b.r), v(b.v) {} // Copy constructor

    void UpdatePosition(Vec const& delta_r) { r += delta_r; };
    void UpdateVelocity(Vec const& delta_v) { v += delta_v; };
private:
    Vec r; // Position vector
    Vec v; // Velocity vector
    std::vector<Boid*> neighbors;
};

#endif
