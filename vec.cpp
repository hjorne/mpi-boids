#include "vec.h"

Vec Vec::operator=(Vec const &rhs)
{
    x = rhs.x;
    y = rhs.y;

    // Return dereferenced pointer curent object
    return *this;
}

Vec Vec::operator+(Vec const &rhs) const
{
    double x_new = x + rhs.x;
    double y_new = y + rhs.y;
    return Vec(x_new, y_new);
}

Vec Vec::operator-(Vec const &rhs) const
{
    double x_new = x - rhs.x;
    double y_new = y - rhs.y;
    return Vec(x_new, y_new);
}

Vec Vec::operator/(double s) const
{
    return Vec(x / s, y / s);
}

Vec Vec::operator*(double s) const
{
    return Vec(x * s, y * s);
}

void Vec::operator+=(Vec const &rhs)
{
    x += rhs.x;
    y += rhs.y;
}

void Vec::operator-=(Vec const &rhs)
{
    x -= rhs.x;
    y -= rhs.y;
}

void Vec::operator/=(double s)
{
    x /= s;
    y /= s;
}

void Vec::operator*=(double s)
{
    x *= s;
    y *= s;
}
