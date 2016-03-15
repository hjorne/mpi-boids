#ifndef _VEC_H_
#define _VEC_H_


typedef struct vec_s {
    double x;
    double y;
} Vec;

void SetAngle(Vec* v, double a);
void SetLength(Vec* v, double l);
void InitRandomAngle(Vec* v, double l);

#endif
