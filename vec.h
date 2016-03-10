#ifndef __VEC_H__
#define __VEC_H__

#include <math.h>

class Vec {
public:
    Vec(double x_=0.0, double y_=0.0) : x(x_), y(y_) {} // Default constructor
    Vec(Vec const& v) : x(v.x), y(v.y) {} // Copy constructor

    double x_get() { return x; }
    double y_get() { return y; }

    double norm() { return sqrt(x*x + y*y); }

    Vec operator=(Vec const&);
    Vec operator+(Vec const&) const;
    Vec operator-(Vec const&) const;
    Vec operator/(double) const;
    Vec operator*(double) const;

    void operator+=(Vec const&);
    void operator-=(Vec const&);
    void operator/=(double);
    void operator*=(double);

private:
    double x;
    double y;
};

#endif
