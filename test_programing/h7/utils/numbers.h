#ifndef NUMBERS_H
#define NUMBERS_H

#include <math.h>
#include <stdlib.h>

#define __Abs(x)    ((x) < 0 ? -(x) : (x))
#define __Max(a, b) ((a) > (b) ? (a) : (b))

union __Uni_Float1{
    float f;
    int val;
};

union __Uni_Double1{
    double f;
    long long val;
};

static inline double __RelDif(double a, double b)
{
    double c = __Abs(a);
    double d = __Abs(b);

    d = __Max(c, d);

    return d == 0.0 ? 0.0 : __Abs(a - b) / d;
}

static inline int isFloatEquals(float a, float b){
    union __Uni_Float1 uf1; uf1.f = a;
    union __Uni_Float1 uf2; uf2.f = b;
    return abs(uf1.val - uf2.val) < 10;
}

static inline int isDoubleEquals(double a, double b){
//    union __Uni_Double1 uf1; uf1.f = a;
//    union __Uni_Double1 uf2; uf2.f = b;
//    return llabs(uf1.val - uf2.val) < 10; // have bug when a, b = 8.88

//    printf("isDoubleEquals >> %g\n", __RelDif(a, b));
    return __RelDif(a, b) <= 1.1e-07;  //1.28875e-08
}

static inline int isFloatEquals2(void* a, void* b){
    union __Uni_Float1 uf1; uf1.f = *(float*)a;
    union __Uni_Float1 uf2; uf2.f = *(float*)b;
    return abs(uf1.val - uf2.val) < 10;
}

static inline int isDoubleEquals2(void* a, void* b){
    return isDoubleEquals(*(double*)a, *(double*)b);
}

#endif // NUMBERS_H
