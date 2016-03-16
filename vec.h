#ifndef _VEC_H_
#define _VEC_H_


typedef struct vec_s {
    double x;
    double y;
} Vec;

void VecRandomAngle(Vec* v, double l);
void VecSetAngle(Vec* v, double a);
void VecSetLength(Vec* v, double l);
double VecAngle(Vec v);
double VecLength(Vec v);


#endif
