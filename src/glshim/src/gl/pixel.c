#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pixel.h"
#include "gl_helpers.h"
#include "gl_str.h"

static const colorlayout_t *get_color_map(GLenum format) {
    #define map(fmt, ...)                               \
        case fmt: {                                     \
        static colorlayout_t layout = {fmt, __VA_ARGS__}; \
        return &layout; }
    switch (format) {
        map(GL_ALPHA, -1, -1, -1, 0);
        map(GL_BGR, 2, 1, 0, -1);
        map(GL_BGRA, 2, 1, 0, 3);
        map(GL_LUMINANCE, 0, 0, 0, -1);
        map(GL_LUMINANCE_ALPHA, 0, 0, 0, 1);
        map(GL_RED, 0, -1, -1, -1);
        map(GL_RG, 0, 1, -1, -1);
        map(GL_RGB, 0, 1, 2, -1);
        map(GL_RGBA, 0, 1, 2, 3);
        default:
            fprintf(stderr, "get_color_map(): Unsupported pixel format %s\n", gl_str(format));
            break;
    }
    static colorlayout_t null = {0};
    return &null;
    #undef map
}

static inline
bool remap_pixel(const GLvoid *src, GLvoid *dst,
                 const colorlayout_t *src_color, GLenum src_type,
                 const colorlayout_t *dst_color, GLenum dst_type) {

    #define type_case(constant, type, ...)        \
        case constant: {                          \
            const type *s = (const type *)src;    \
            type *d = (type *)dst;                \
            type v = *s;                          \
            __VA_ARGS__                           \
            break;                                \
        }

    #define default(arr, amod, vmod, key, def) \
        key >= 0 ? arr[amod key] vmod : def

    #define carefully(arr, amod, key, value) \
        if (key >= 0) d[amod key] = value;

    #define read_each(amod, vmod)                                 \
        pixel.r = default(s, amod, vmod, src_color->red, 0);      \
        pixel.g = default(s, amod, vmod, src_color->green, 0);    \
        pixel.b = default(s, amod, vmod, src_color->blue, 0);     \
        pixel.a = default(s, amod, vmod, src_color->alpha, 1.0f);

    #define write_each(amod, vmod)                         \
        carefully(d, amod, dst_color->red, pixel.r vmod)   \
        carefully(d, amod, dst_color->green, pixel.g vmod) \
        carefully(d, amod, dst_color->blue, pixel.b vmod)  \
        carefully(d, amod, dst_color->alpha, pixel.a vmod)

    // this pixel stores our intermediate color
    // it will be RGBA and normalized to between (0.0 - 1.0f)
    pixel_t pixel;
    switch (src_type) {
        type_case(GL_DOUBLE, GLdouble, read_each(,))
        type_case(GL_FLOAT, GLfloat, read_each(,))
        case GL_UNSIGNED_INT_8_8_8_8_REV:
        type_case(GL_UNSIGNED_BYTE, GLubyte, read_each(, / 255.0f))
        type_case(GL_UNSIGNED_INT_8_8_8_8, GLubyte, read_each(3 - , / 255.0f))
        type_case(GL_UNSIGNED_SHORT_4_4_4_4, GLushort,
            s = (GLushort[]){
                (v >> 0)  & 0x0f,
                (v >> 4)  & 0x0f,
                (v >> 8)  & 0x0f,
                (v >> 12) & 0x0f,
            };
            read_each(, / 15.0f);
        )
        type_case(GL_UNSIGNED_SHORT_5_5_5_1, GLushort,
            s = (GLushort[]){
                ((v & 0x8000) >> 15) * 31,
                ((v & 0x7c00) >> 10),
                ((v & 0x03e0) >> 5),
                v & 31,
            };
            read_each(, / 31.0f);
        )
        type_case(GL_UNSIGNED_SHORT_1_5_5_5_REV, GLushort,
            s = (GLushort[]){
                v & 31,
                ((v & 0x03e0) >> 5),
                ((v & 0x7c00) >> 10),
                ((v & 0x8000) >> 15) * 31,
            };
            read_each(, / 31.0f);
        )
        default:
            // TODO: add glSetError?
            fprintf(stderr, "remap_pixel(): Unsupported source data type: %s\n", gl_str(src_type));
            return false;
            break;
    }

    switch (dst_type) {
        type_case(GL_FLOAT, GLfloat, write_each(,))
        type_case(GL_UNSIGNED_BYTE, GLubyte, write_each(, * 255.0))
        // TODO: force 565 to RGB? then we can change [4] -> 3
        type_case(GL_UNSIGNED_SHORT_5_6_5, GLushort,
            GLfloat color[3];
            color[dst_color->red] = pixel.r;
            color[dst_color->green] = pixel.g;
            color[dst_color->blue] = pixel.b;
            *d = (((GLuint)(color[0] * 31) & 0x1f) << 11) |
                 (((GLuint)(color[1] * 63) & 0x3f) << 5) |
                 (((GLuint)(color[2] * 31) & 0x1f));
        )
        type_case(GL_UNSIGNED_SHORT_5_5_5_1, GLushort,
            GLfloat color[4];
            color[dst_color->red] = pixel.r;
            color[dst_color->green] = pixel.g;
            color[dst_color->blue] = pixel.b;
            color[dst_color->alpha] = pixel.a;
            // TODO: can I macro this or something? it follows a pretty strict form.
            *d = (((GLuint)(color[0] * 31) & 0x1f) << 0) |
                 (((GLuint)(color[1] * 31) & 0x1f) << 5) |
                 (((GLuint)(color[2] * 31) & 0x1f) << 10)  |
                 (((GLuint)(color[3] * 1)  & 0x01) << 15);
        )
       type_case(GL_UNSIGNED_SHORT_4_4_4_4, GLushort,
            GLfloat color[4];
            color[dst_color->red] = pixel.r;
            color[dst_color->green] = pixel.g;
            color[dst_color->blue] = pixel.b;
            color[dst_color->alpha] = pixel.a;
            *d = (((GLushort)(color[0] * 15) & 0x0f) << 12) |
                 (((GLushort)(color[1] * 15) & 0x0f) << 8) |
                 (((GLushort)(color[2] * 15) & 0x0f) << 4) |
                 (((GLushort)(color[3] * 15) & 0x0f));
        )
        default:
            fprintf(stderr, "remap_pixel(): Unsupported target data type: %s\n", gl_str(dst_type));
            return false;
            break;
    }
    return true;

    #undef type_case
    #undef default
    #undef carefully
    #undef read_each
    #undef write_each
}

bool pixel_convert_direct(const GLvoid *src, GLvoid *dst, GLuint width,
                          GLenum src_format, GLenum src_type, GLsizei src_stride,
                          GLenum dst_format, GLenum dst_type, GLsizei dst_stride) {
    const colorlayout_t *src_color, *dst_color;
    src_color = get_color_map(src_format);
    dst_color = get_color_map(dst_format);

    uintptr_t src_pos = (uintptr_t)src;
    uintptr_t dst_pos = (uintptr_t)dst;
    for (int i = 0; i < width; i++) {
        if (! remap_pixel((const GLvoid *)src_pos, (GLvoid *)dst_pos,
                          src_color, src_type, dst_color, dst_type)) {
            // checking a boolean for each pixel like this might be a slowdown?
            // probably depends on how well branch prediction performs
            return false;
        }
        src_pos += src_stride;
        dst_pos += dst_stride;
    }
    return true;
}

bool pixel_convert(const GLvoid *src, GLvoid **dst,
                   GLuint width, GLuint height,
                   GLenum src_format, GLenum src_type,
                   GLenum dst_format, GLenum dst_type) {
    const colorlayout_t *src_color, *dst_color;
    GLuint pixels = width * height;
    GLuint dst_size = pixels * gl_pixel_sizeof(dst_format, dst_type);

    src_color = get_color_map(src_format);
    dst_color = get_color_map(dst_format);
    if (!dst_size || !gl_pixel_sizeof(src_format, src_type)
        || !src_color->type || !dst_color->type)
        return false;

    if (src_type == dst_type && src_color->type == dst_color->type) {
        if (*dst != src) {
            *dst = malloc(dst_size);
            memcpy(*dst, src, dst_size);
            return true;
        }
    } else {
        GLsizei src_stride = gl_pixel_sizeof(src_format, src_type);
        GLsizei dst_stride = gl_pixel_sizeof(dst_format, dst_type);
        *dst = malloc(dst_size);
        return pixel_convert_direct(
            src, *dst, pixels,
            src_format, src_type, src_stride,
            dst_format, dst_type, dst_stride);
    }
    return false;
}

bool pixel_scale(const GLvoid *old, GLvoid **new,
                 GLuint width, GLuint height,
                 GLfloat ratio,
                 GLenum format, GLenum type) {
    GLuint pixel_size, new_width, new_height;
    new_width = width * ratio;
    new_height = height * ratio;
    fprintf(stderr, "scaling %ux%u -> %ux%u\n", width, height, new_width, new_height);
    GLvoid *dst;
    uintptr_t src, pos, pixel;

    pixel_size = gl_pixel_sizeof(format, type);
    dst = malloc(pixel_size * new_width * new_height);
    src = (uintptr_t)old;
    pos = (uintptr_t)dst;
    for (int x = 0; x < new_width; x++) {
        for (int y = 0; y < new_height; y++) {
            pixel = src + (x / ratio) +
                          (y / ratio) * width;
            memcpy((GLvoid *)pos, (GLvoid *)pixel, pixel_size);
            pos += pixel_size;
        }
    }
    *new = dst;
    return true;
}

bool pixel_to_ppm(const GLvoid *pixels, GLuint width, GLuint height,
                  GLenum format, GLenum type, GLuint name) {
    if (! pixels)
        return false;

    const GLvoid *src;
    char filename[64];
    int size = 4 * 3 * width * height;
    if (format == GL_RGB && type == GL_UNSIGNED_BYTE) {
        src = pixels;
    } else {
        if (! pixel_convert(pixels, (GLvoid **)&src, width, height, format, type, GL_RGB, GL_UNSIGNED_BYTE)) {
            return false;
        }
    }

    snprintf(filename, 64, "/tmp/tex.%d.ppm", name);
    FILE *fd = fopen(filename, "w");
    fprintf(fd, "P6 %d %d %d\n", width, height, 255);
    fwrite(src, 1, size, fd);
    fclose(fd);
    return true;
}
