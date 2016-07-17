#include <stdio.h>

#include "block.h"
#include "error.h"
#include "texgen.h"
#include "vectorial/simd4f.h"
#include "vectorial/simd4x4f.h"

void glTexGeni(GLenum coord, GLenum pname, GLint param) {
    GLfloat fp = param;
    glTexGenfv(coord, pname, &fp);
}

void glTexGenfv(GLenum coord, GLenum pname, const GLfloat *param) {
    // pname is in: GL_TEXTURE_GEN_MODE, GL_OBJECT_PLANE, GL_EYE_PLANE
    ERROR_IN_BLOCK();

    texgen_state_t *texgen = &state.texgen[state.texture.active];
    if (pname == GL_TEXTURE_GEN_MODE) {
        switch ((GLenum)*param) {
            case GL_SPHERE_MAP:
                if (coord == GL_R || coord == GL_Q) {
                    ERROR(GL_INVALID_ENUM);
                }
            case GL_OBJECT_LINEAR:
            case GL_EYE_LINEAR:
            // TODO: missing GL_NORMAL_MAP implementation
            case GL_NORMAL_MAP:
            case GL_REFLECTION_MAP:
                break;
            default:
                ERROR(GL_INVALID_ENUM);
        }
        switch (coord) {
            case GL_R: texgen->R = *param; break;
            case GL_Q: texgen->Q = *param; break;
            case GL_S: texgen->S = *param; break;
            case GL_T: texgen->T = *param; break;
        }
    } else {
        GLfloat *target = NULL;
        switch (coord) {
            case GL_R:
                target = texgen->Rv;
                break;
            case GL_Q:
                target = texgen->Qv;
                break;
            case GL_S:
                target = texgen->Sv;
                break;
            case GL_T:
                target = texgen->Tv;
                break;
            default:
                ERROR(GL_INVALID_ENUM);
                return;
        }
        memcpy(target, param, 4 * sizeof(GLfloat));
    }
}

static inline void tex_coord_loop(block_t *block, GLfloat *out, GLenum type, GLfloat *Sp, GLfloat *Tp) {
    GLfloat *vert = block->vert;
    GLfloat *normal = block->normal;
    // if we get sphere map and no normal, just barf and return?

    simd4x4f matrix;
    if (type != GL_OBJECT_LINEAR) {
        // TODO: make sure this loads it properly
        glGetFloatv(GL_MODELVIEW_MATRIX, (float *)&matrix);
    }
    simd4f s_plane, t_plane;
    s_plane = simd4f_uload4(Sp);
    if (Tp != NULL) {
        t_plane = simd4f_uload4(Tp);
    }
    for (int i = 0; i < block->len; i++) {
        if (block->indices) {
            vert = block->vert + (block->indices[i]) * 3;
        }
        if (! block->normal) {
            normal = CURRENT->normal;
        }
        GLfloat tmp[2];
        simd4f v = simd4f_create(vert[0], vert[1], vert[2], 1);
        switch (type) {
            case GL_OBJECT_LINEAR:
                simd4f_ustore2(simd4f_dot4(v, s_plane), tmp);
                out[0] = tmp[0];
                if (Tp) {
                    simd4f_ustore2(simd4f_dot4(v, t_plane), tmp);
                    out[1] = tmp[0];
                }
                break;
            case GL_EYE_LINEAR: {
                simd4f eye;
                simd4x4f_matrix_vector_mul(&matrix, &v, &eye);
                simd4f_ustore2(simd4f_dot4(eye, s_plane), tmp);
                out[0] = tmp[0];
                if (Tp) {
                    simd4f_ustore2(simd4f_dot4(eye, t_plane), tmp);
                    out[1] = tmp[0];
                }
                break;
            }
            case GL_SPHERE_MAP: {
                simd4f norm = simd4f_create(normal[0], normal[1], normal[2], 1.0f);
                simd4f eye;
                simd4x4f_matrix_vector_mul(&matrix, &v, &eye);
                eye = simd4f_normalize3(eye);
                simd4f eye_normal;

                simd4x4f inverse;
                simd4x4f_inverse(&matrix, &inverse);
                // TODO: better to use new registers to prevent stall?
                simd4x4f_transpose_inplace(&inverse);
                // TODO: is normal multiplied wrong here?
                simd4x4f_matrix_vector_mul(&inverse, &norm, &eye_normal);
                simd4f result = simd4f_mul(simd4f_dot4(eye, eye_normal), simd4f_create(2.0f, 2.0f, 2.0f, 2.0f));
                simd4f reflection = simd4f_sub(eye, simd4f_mul(eye_normal, result));
                reflection = simd4f_add(reflection, simd4f_create(0.0f, 0.0f, 1.0f, 0.0f));
                float ref[4], dot[2];
                simd4f_ustore2(reflection, ref);
                simd4f_ustore2(simd4f_dot4(reflection, reflection), dot);
                float m = 1.0 / (2.0 * sqrt(dot[0]));
                out[0] = ref[0] * m + 0.5;
                out[1] = ref[1] * m + 0.5;

                normal += 3;
                break;
            }
            case GL_REFLECTION_MAP: {
                float eye_[2], eye_normal_[2], dot[2];
                simd4f norm = simd4f_create(normal[0], normal[1], normal[2], 1.0f);
                simd4f eye;
                simd4x4f_matrix_vector_mul(&matrix, &v, &eye);
                eye = simd4f_normalize3(eye);
                simd4f_ustore2(eye, eye_);

                simd4f eye_normal;
                simd4x4f inverse;
                simd4x4f_inverse(&matrix, &inverse);
                simd4x4f_transpose_inplace(&inverse);
                simd4x4f_matrix_vector_mul(&inverse, &norm, &eye_normal);
                simd4f_ustore2(eye_normal, eye_);

                simd4f_ustore2(simd4f_dot4(eye, eye_normal), dot);
                out[0] = eye_[0] - eye_normal_[0] * dot[0] * 2.0f;
                out[1] = eye_[1] - eye_normal_[1] * dot[0] * 2.0f;
                // out[2] = eye.x - eye_normal.z * dot;
                break;
            }
        }
        out += 2;
        vert += 3;
    }
}

void gen_tex_coords(block_t *block, GLuint texture) {
    // TODO: do less work when called from glDrawElements?

    block->tex[texture] = (GLfloat *)malloc(block->len * 2 * sizeof(GLfloat));
    texgen_state_t *texgen = &state.texgen[texture];
    if (state.enable.texgen_s[texture]) {
        if (texgen->S == texgen->T) {
            tex_coord_loop(block, block->tex[texture], texgen->S, texgen->Sv, texgen->Tv);
        } else {
            tex_coord_loop(block, block->tex[texture], texgen->S, texgen->Sv, NULL);
        }
    }
    if (state.enable.texgen_t[texture]) {
        tex_coord_loop(block, block->tex[texture] + 1, texgen->T, texgen->Tv, NULL);
    }
}
