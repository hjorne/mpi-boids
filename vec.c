#include "clcg4.h"
#include "vec.h"
#include <stdio.h>
#include <math.h>
#include <time.h>

/* Generates a vector with a random angle and length l */
void
VecRandomAngle(Vec* v, double l)
{
    srand( time(NULL) );
    int seed = rand() % Maxgen;
    double a = GenVal(seed) * 2 * M_PI;
    v->x = l * cos(a);
    v->y = l * sin(a);
}

/* Sets the angle of a vector without changing its length */
void
VecSetAngle(Vec* v, double a)
{
    double len = VecLength(*v);
    v->x = len * cos(a);
    v->y = len * sin(a);
}

/* Sets the length of a vector without changing its angle */
void
VecSetLength(Vec* v, double l)
{
    double len = VecLength(*v);
    v->x *= l / len;
    v->y *= l / len;
}

/* Returns the angle of a vector from -pi to pi */
double
VecAngle(Vec v)
{
    return atan2(v.y, v.x);
}

/* Returns the length of a vector */
double
VecLength(Vec v)
{
    return sqrt(v.x * v.x + v.y * v.y);
}
