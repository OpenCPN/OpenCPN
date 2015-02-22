#include "array.h"

GLvoid *copy_gl_array(const GLvoid *src,
                      GLenum from, GLsizei width, GLsizei stride,
                      GLenum to, GLsizei to_width, GLsizei skip, GLsizei count) {
    if (! src || !count)
        return NULL;

    if (! stride)
        stride = width * gl_sizeof(from);

    const char *unknown_str = "libGL: copy_gl_array -> unknown type: %x\n";
    GLvoid *dst = malloc(count * to_width * gl_sizeof(to));
    GLsizei from_size = gl_sizeof(from) * width;
    GLsizei to_size = gl_sizeof(to) * to_width;

    if (to_width < width) {
        printf("Warning: copy_gl_array: %i < %i\n", to_width, width);
        return NULL;
    }

    // if stride is weird, we need to be able to arbitrarily shift src
    // so we leave it in a uintptr_t and cast after incrementing
    uintptr_t in = (uintptr_t)src;
    if (from == to && to_width >= width) {
        GL_TYPE_SWITCH(out, dst, to,
            for (int i = skip; i < count; i++) {
                memcpy(out, (GLvoid *)in, from_size);
                for (int j = width; j < to_width; j++) {
                    out[j] = 0;
                }
                out += to_width;
                in += stride;
            },
            default:
                printf(unknown_str, from);
                return NULL;
        )
    } else {
        GL_TYPE_SWITCH(out, dst, to,
            for (int i = skip; i < count; i++) {
                GL_TYPE_SWITCH(input, in, from,
                    for (int j = 0; j < width; j++) {
                        out[j] = input[j];
                    }
                    for (int j = width; j < to_width; j++) {
                        out[j] = 0;
                    }
                    out += to_width;
                    in += stride;
                ,
                    default:
                        printf(unknown_str, from);
                        return NULL;
                )
            },
            default:
                printf(unknown_str, to);
                return NULL;
        )
    }

    return dst;
}

GLvoid *copy_gl_pointer(pointer_state_t *ptr, GLsizei width, GLsizei skip, GLsizei count) {
    return copy_gl_array(ptr->pointer, ptr->type, ptr->size, ptr->stride,
                         GL_FLOAT, width, skip, count);
}

GLfloat *gl_pointer_index(pointer_state_t *p, GLint index) {
    static GLfloat buf[4];
    GLsizei size = gl_sizeof(p->type);
    GLsizei stride = p->stride ? p->stride : size * p->size;
    uintptr_t ptr = (uintptr_t)p->pointer + (stride * index);

    GL_TYPE_SWITCH(src, ptr, p->type,
        for (int i = 0; i < p->size; i++) {
            buf[i] = src[i];
        }
        // zero anything not set by the pointer
        for (int i = p->size; i < 4; i++) {
            buf[i] = 0;
        },
        default:
            printf("libGL: unknown pointer type: 0x%x\n", p->type);
    )
    return buf;
}


GLfloat *copy_eval_double(GLenum target, GLint ustride, GLint uorder,
                          GLint vstride, GLint vorder,
                          const GLdouble *src) {

    GLsizei width = get_map_width(target);
    GLsizei dwidth = (uorder == 2 && vorder == 2) ? 0 : uorder * vorder;
    GLsizei hwidth = (uorder > vorder ? uorder : vorder) * width;
    GLsizei elements;
    GLsizei uinc = ustride - vorder * vstride;

    if (hwidth > dwidth) {
        elements = (uorder * vorder * width + hwidth);
    } else {
        elements = (uorder * vorder * width + dwidth);
    }
    GLfloat *points = malloc(elements * sizeof(GLfloat));
    GLfloat *dst = points;

    for (int i = 0; i < uorder; i++, src += uinc) {
        for (int j = 0; j < vorder; j++, src += vstride) {
            for (int k = 0; k < width; k++) {
                *dst++ = src[k];
            }
        }
    }
    return points;
}

void normalize_indices(GLushort *indices, GLsizei *max, GLsizei *min, GLsizei count) {
    *max = 0;
    *min = -1;
    for (int i = 0; i < count; i++) {
        GLsizei n = indices[i];
        if (*min == -1)
            *min = n;
        *min = (n < *min) ? n : *min;
        *max = (n > *max) ? n : *max;
    }
    for (int i = 0; i < count; i++) {
        indices[i] -= *min;
    }
}
