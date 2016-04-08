#ifndef _VEC_H_
#define _VEC_H_

/* Simple vector for 2D only */
typedef struct vec_s {
    double x;
    double y;
} Vec;

/* Returns the angle of a vector from -pi to pi */
double VecAngle(Vec v);

/* Returns the length of a vector */
double VecLength(Vec v);

/* Sets the angle of a vector without changing its length */
void VecSetAngle(Vec* v, double a);

/* Sets the length of a vector without changing its angle */
void VecSetLength(Vec* v, double l);

/* Generates a vector with a random angle and length l */
void VecRandomAngle(Vec* v, double l);


#endif
