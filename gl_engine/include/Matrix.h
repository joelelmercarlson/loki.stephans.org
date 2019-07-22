#ifndef MATRIX_H
#define MATRIX_H

#include <cmath>
#include <iostream>

// c style
// tbd full c++ class

typedef float MATRIX3[9];
typedef float MATRIX4[16];
const MATRIX3 M3_I = { 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f };
const MATRIX4 M4_I = { 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f };
const MATRIX3 M3_Z = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };
const MATRIX4 M4_Z = { 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f };

extern float m3_det(MATRIX3 mat);
extern float m4_det(MATRIX4 mat);
extern int m3_inverse(MATRIX3 mr, MATRIX3 ma);
extern void m4_zero(MATRIX4 mat);
extern void m4_identity(MATRIX4 mat);
extern void m4_copy(MATRIX4 mr, const MATRIX4 mb);
extern void m4_submat(MATRIX4 mr, MATRIX3 mb, int i, int j);
extern int m4_inverse(MATRIX4 mr, MATRIX4 ma);
extern void m4_print(const MATRIX4 mat);
#endif
