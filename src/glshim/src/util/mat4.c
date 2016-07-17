#include "mat4.h"

mat4 mat4_new() {
    mat4 out;
    mat4_identity(&out);
    return out;
}

void mat4_transpose(mat4 *m) {
    simd4x4f_transpose_inplace(m);
}

void mat4_identity(mat4 *m) {
    simd4x4f_identity(m);
}

void mat4_load(mat4 *m, const float *load) {
    simd4x4f_uload(m, load);
}

void mat4_save(mat4 *m, float *out) {
    simd4x4f_ustore(m, out);
}

void mat4_mul(mat4 *m, mat4 *quotient) {
    simd4x4f out;
    simd4x4f_matrix_mul(m, quotient, &out);
    *m = out;
}

void mat4_rotate(mat4 *m, float angle, float x, float y, float z) {
    float radians = angle * VECTORIAL_PI / 180;
    simd4x4f rotate;
    simd4x4f_axis_rotation(&rotate, radians, simd4f_create(x, y, z, 1.0f));
    mat4_mul(m, &rotate);
}

void mat4_scale(mat4 *m, float x, float y, float z) {
    simd4x4f scale;
    simd4x4f_scaling(&scale, x, y, z);
    mat4_mul(m, &scale);
}

void mat4_translate(mat4 *m, float x, float y, float z) {
    simd4x4f translate;
    simd4x4f_translation(&translate, x, y, z);
    mat4_mul(m, &translate);
}

void mat4_ortho(mat4 *m, float left, float right,
                          float bottom, float top,
                          float near, float far) {
    simd4x4f ortho;
    simd4x4f_ortho(&ortho, left, right, bottom, top, near, far);
    mat4_mul(m, &ortho);
}

void mat4_frustum(mat4 *m, float left, float right,
                           float bottom, float top,
                           float near, float far) {
    simd4x4f frustum;
    simd4x4f_frustum(&frustum, left, right, bottom, top, near, far);
    mat4_mul(m, &frustum);
}

void mat4_perspective(mat4 *m, float fov, float aspect, float znear, float zfar) {
    simd4x4f perspective;
    simd4x4f_perspective(&perspective, fov, aspect, znear, zfar);
    mat4_mul(m, &perspective);
}

void mat4_mul_vec2(mat4 *m, float out[2], const float in[2]) {
    simd4f tmp, vert = simd4f_create(in[0], in[1], 0.0f, 1.0f);
    simd4x4f_matrix_vector_mul(m, &vert, &tmp);
    tmp = simd4f_div(tmp, simd4f_splat_w(tmp));
    simd4f_ustore2(tmp, out);
}

void mat4_mul_vec3(mat4 *m, float out[3], const float in[3]) {
    simd4f tmp, vert = simd4f_create(in[0], in[1], in[2], 1.0f);
    simd4x4f_matrix_vector_mul(m, &vert, &tmp);
    tmp = simd4f_div(tmp, simd4f_splat_w(tmp));
    simd4f_ustore3(tmp, out);
}

void mat4_mul_vec4(mat4 *m, float out[4], const float in[4]) {
    simd4f tmp, vert = simd4f_create(in[0], in[1], in[2], in[3]);
    simd4x4f_matrix_vector_mul(m, &vert, &tmp);
    simd4f_ustore4(tmp, out);
}
