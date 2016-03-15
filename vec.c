#include <math.h>
#include <stdio.h>
#include "clcg4.h"
#include "vec.h"

void VecSetAngle(Vec* v, double a)
{

}

void VecSetLength(Vec* v, double l)
{

}


void VecRandomAngle(Vec* v, double l)
{
    int seed = 1;
    double a = GenVal(seed) * 2 * M_PI;
    v->x = l * sin(a);
    v->y = l * cos(a);
}
