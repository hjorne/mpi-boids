#include <math.h>
#include <stdio.h>
#include "clcg4.h"
#include "vec.h"

void VecRandomAngle(Vec* v, double l)
{
    int seed = 1;
    double a = GenVal(seed) * 2 * M_PI;
    v->x = l * cos(a);
    v->y = l * sin(a);
}


void VecSetAngle(Vec* v, double a)
{
    double len = VecLength(*v);
    v->x = len * cos(a);
    v->y = len * sin(a);
}

void VecSetLength(Vec* v, double l)
{
    double len = VecLength(*v);
    v->x *= l / len;
    v->y *= l / len;
}

double VecAngle(Vec v)
{
    return atan2(v.y, v.x);
}

double VecLength(Vec v)
{
    return sqrt(v.x * v.x + v.y * v.y);
}
