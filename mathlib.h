#ifndef _MATH_LIB_H
#define _MATH_LIB_H

#include "lib.h"

// Started an integer math library.
// x^y
int power(int x, int y);
// absolute value
int abs(int x);

// Array mass-operation functions:
// First n elem = x
void setNInt(int32_t * array, int32_t n, int32_t x);
// [a,b] = x
void blkSetInt(int32_t * array, int32_t a, int32_t b, int32_t x);

#endif
