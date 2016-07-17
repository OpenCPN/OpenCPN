#ifndef GPU_MAT4_H
#define GPU_MAT4_H

#include "vectorial/simd4f.h"
#include "vectorial/simd4x4f.h"

typedef simd4x4f mat4;

mat4 mat4_new();
void mat4_transpose(mat4 *m);
void mat4_identity(mat4 *m);
void mat4_clone(mat4 *m);
void mat4_load(mat4 *m, const float *load);
void mat4_save(mat4 *m, float *out);
void mat4_mul(mat4 *m, mat4 *quotient);
void mat4_rotate(mat4 *m, float angle, float x, float y, float z);
void mat4_scale(mat4 *m, float x, float y, float z);
void mat4_translate(mat4 *m, float x, float y, float z);
void mat4_ortho(mat4 *m, float left, float right, float bottom, float top, float near, float far);
void mat4_frustum(mat4 *m, float left, float right, float bottom, float top, float near, float far);
void mat4_perspective(mat4 *m, float fov, float aspect, float znear, float zfar);
void mat4_mul_vec2(mat4 *m, float out[2], const float in[2]);
void mat4_mul_vec3(mat4 *m, float out[3], const float in[3]);
void mat4_mul_vec4(mat4 *m, float out[4], const float in[4]);

#endif
