#ifndef _VEC_H_
#define _VEC_H_


typedef struct vec_s {
    double x;
    double y;
} Vec;




void VecRandomAngle(Vec* v, double l);
void VecSetLength(Vec* v, double l);
void VecSetAngle(Vec* v, double a);
double VecLength(Vec v);
double VecAngle(Vec v);



#endif
