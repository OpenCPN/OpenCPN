#include "texture.h"

// expand non-power-of-two sizes
// TODO: what does this do to repeating textures?
int npot(int n) {
    if (n == 0) return 0;

    int i = 1;
    while (i < n) i <<= 1;
    return i;
}

// conversions for GL_ARB_texture_rectangle
void tex_coord_rect_arb(GLfloat *tex, GLsizei len,
                        GLsizei width, GLsizei height) {
    if (!tex || !width || !height)
        return;

    for (int i = 0; i < len; i++) {
        tex[0] /= width;
        tex[1] /= height;
        tex += 2;
    }
}

void tex_coord_npot(GLfloat *tex, GLsizei len,
                    GLsizei width, GLsizei height,
                    GLsizei nwidth, GLsizei nheight) {
    if (!tex || !width || !height)
        return;

    GLfloat wratio = (width / (GLfloat)nwidth);
    GLfloat hratio = (height / (GLfloat)nheight);
    for (int i = 0; i < len; i++) {
        tex[0] *= wratio;
        tex[1] *= hratio;
        tex += 2;
    }
}

static void *swizzle_texture(GLsizei width, GLsizei height,
                             GLenum *format, GLenum *type,
                             const GLvoid *data) {
    bool convert = false;
    switch (*format) {
        case GL_ALPHA:
        case GL_RGB:
        case GL_RGBA:
        case GL_LUMINANCE:
        case GL_LUMINANCE_ALPHA:
            break;
        default:
            convert = true;
            break;
    }
    switch (*type) {
        case GL_FLOAT:
        case GL_UNSIGNED_BYTE:
        case GL_UNSIGNED_SHORT_5_6_5:
        case GL_UNSIGNED_SHORT_4_4_4_4:
        case GL_UNSIGNED_SHORT_5_5_5_1:
            break;
        case GL_UNSIGNED_INT_8_8_8_8_REV:
            *type = GL_UNSIGNED_BYTE;
            break;
        default:
            convert = true;
            break;
    }

    if (convert) {
        GLvoid *pixels = (GLvoid *)data;
        if (! pixel_convert(data, &pixels, width, height,
                            *format, *type, GL_RGBA, GL_UNSIGNED_BYTE)) {
            printf("libGL swizzle error: (%#4x, %#4x -> RGBA, UNSIGNED_BYTE)\n",
                *format, *type);
            return NULL;
        }
        *type = GL_UNSIGNED_BYTE;
        *format = GL_RGBA;
        return pixels;
    }
    return (void *)data;
}

void glTexImage2D(GLenum target, GLint level, GLint internalFormat,
                  GLsizei width, GLsizei height, GLint border,
                  GLenum format, GLenum type, const GLvoid *data) {

    gltexture_t *bound = state.texture.bound;
    GLvoid *pixels = (GLvoid *)data;
    if (data) {
        // implements GL_UNPACK_ROW_LENGTH
        if (state.texture.unpack_row_length && state.texture.unpack_row_length != width) {
            int imgWidth, pixelSize;
            pixelSize = pixel_sizeof(format, type);
            imgWidth = state.texture.unpack_row_length * pixelSize;
            GLubyte *dst = (GLubyte *)malloc(width * height * pixelSize);
            pixels = (GLvoid *)dst;
            const GLubyte *src = (GLubyte *)data;
            src += state.texture.unpack_skip_pixels + state.texture.unpack_skip_rows * imgWidth;
            for (int y = 0; y < height; y += 1) {
                memcpy(dst, src, width * pixelSize);
                src += imgWidth;
                dst += width;
            }
        }

        GLvoid *old = pixels;
        pixels = (GLvoid *)swizzle_texture(width, height, &format, &type, data);
        if (old != pixels && old != data)
            free(old);

        char *env_shrink = getenv("LIBGL_SHRINK");
        if (env_shrink && strcmp(env_shrink, "1") == 0) {
            if (width > 1 && height > 1) {
                GLvoid *out;
                GLfloat ratio = 0.5;
                pixel_scale(pixels, &out, width, height, ratio, format, type);
                if (out != pixels)
                    free(out);
                pixels = out;
                width *= ratio;
                height *= ratio;
            }
        }

        char *env_dump = getenv("LIBGL_TEXDUMP");
        if (env_dump && strcmp(env_dump, "1") == 0) {
            if (bound) {
                pixel_to_ppm(pixels, width, height, format, type, bound->texture);
            }
        }
    }

    /* TODO:
    GL_INVALID_VALUE is generated if border is not 0.
    GL_INVALID_OPERATION is generated if type is
    GL_UNSIGNED_SHORT_5_6_5 and format is not GL_RGB.

    GL_INVALID_OPERATION is generated if type is one of
    GL_UNSIGNED_SHORT_4_4_4_4, or GL_UNSIGNED_SHORT_5_5_5_1
    and format is not GL_RGBA.
    */

    LOAD_GLES(glTexImage2D);
    LOAD_GLES(glTexSubImage2D);

    switch (target) {
        case GL_PROXY_TEXTURE_2D:
            break;
        default: {
            GLsizei nheight = npot(height), nwidth = npot(width);
            if (bound && level == 0) {
                bound->width = width;
                bound->height = height;
                bound->nwidth = nwidth;
                bound->nheight = nheight;
            }
            if (height != nheight || width != nwidth) {
                gles_glTexImage2D(target, level, format, nwidth, nheight, border,
                                  format, type, NULL);
                gles_glTexSubImage2D(target, level, 0, 0, width, height,
                                     format, type, pixels);
            } else {
                gles_glTexImage2D(target, level, internalFormat, width, height, border,
                                  format, type, pixels);
            }
        }
    }
    if (pixels != data) {
        free(pixels);
    }
}

void glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset,
                     GLsizei width, GLsizei height, GLenum format, GLenum type,
                     const GLvoid *data) {
    LOAD_GLES(glTexSubImage2D);
    target = map_tex_target(target);
    const GLvoid *pixels = swizzle_texture(width, height, &format, &type, data);
    gles_glTexSubImage2D(target, level, xoffset, yoffset,
                         width, height, format, type, pixels);
    if (pixels != data)
        free((GLvoid *)pixels);
}

// 1d stubs
void glTexImage1D(GLenum target, GLint level, GLint internalFormat,
                  GLsizei width, GLint border,
                  GLenum format, GLenum type, const GLvoid *data) {

    // TODO: maybe too naive to force GL_TEXTURE_2D here?
    glTexImage2D(GL_TEXTURE_2D, level, internalFormat, width, 1,
                 border, format, type, data);
}
void glTexSubImage1D(GLenum target, GLint level, GLint xoffset, GLint yoffset,
                     GLsizei width, GLenum format, GLenum type,
                     const GLvoid *data) {

    glTexSubImage2D(target, level, xoffset, yoffset,
                    width, 1, format, type, data);
}

// 3d stubs
void glTexImage3D(GLenum target, GLint level, GLint internalFormat,
                  GLsizei width, GLsizei height, GLsizei depth, GLint border,
                  GLenum format, GLenum type, const GLvoid *data) {

    // TODO: maybe too naive to force GL_TEXTURE_2D here?
    glTexImage2D(GL_TEXTURE_2D, level, internalFormat, width, height,
                 border, format, type, data);
}
void glTexSubImage3D(GLenum target, GLint level, GLint xoffset, GLint yoffset,
                     GLsizei width, GLsizei height, GLsizei depth, GLenum format,
                     GLenum type, const GLvoid *data) {

    glTexSubImage2D(target, level, xoffset, yoffset,
                    width, height, format, type, data);
}

void glPixelStorei(GLenum pname, GLint param) {
    // TODO: add to glGetIntegerv?
    LOAD_GLES(glPixelStorei);
    switch (pname) {
        case GL_UNPACK_ROW_LENGTH:
            state.texture.unpack_row_length = param;
            break;
        case GL_UNPACK_SKIP_PIXELS:
            state.texture.unpack_skip_pixels = param;
            break;
        case GL_UNPACK_SKIP_ROWS:
            state.texture.unpack_skip_rows = param;
            break;
        case GL_UNPACK_LSB_FIRST:
            state.texture.unpack_lsb_first = param;
            break;
        default:
            gles_glPixelStorei(pname, param);
            break;
    }
}

void glBindTexture(GLenum target, GLuint texture) {
    if (state.list.active) {
        rlBindTexture(state.list.active, texture);
    } else {
        if (texture) {
            int ret;
            khint_t k;
            khash_t(tex) *list = state.texture.list;
            if (! list) {
                list = state.texture.list = kh_init(tex);
                // segfaults if we don't do a single put
                kh_put(tex, list, 1, &ret);
                kh_del(tex, list, 1);
            }

            k = kh_get(tex, list, texture);
            gltexture_t *tex = NULL;;
            if (k == kh_end(list)){
                k = kh_put(tex, list, texture, &ret);
                tex = kh_value(list, k) = malloc(sizeof(gltexture_t));
                tex->texture = texture;
                tex->target = target;
                tex->width = 0;
                tex->height = 0;
                tex->uploaded = false;
            } else {
                tex = kh_value(list, k);
            }
            state.texture.bound = tex;
        } else {
            state.texture.bound = NULL;
        }

        state.texture.rect_arb = (target == GL_TEXTURE_RECTANGLE_ARB);
        target = map_tex_target(target);

        LOAD_GLES(glBindTexture);
        gles_glBindTexture(target, texture);
    }
}

// TODO: also glTexParameterf(v)?
void glTexParameteri(GLenum target, GLenum pname, GLint param) {
    PUSH_IF_COMPILING(glTexParameteri);
    LOAD_GLES(glTexParameteri);
    target = map_tex_target(target);
    switch (param) {
        case GL_CLAMP:
            param = GL_CLAMP_TO_EDGE;
            break;
    }
    gles_glTexParameteri(target, pname, param);
}

void glDeleteTextures(GLsizei n, const GLuint *textures) {
    PUSH_IF_COMPILING(glDeleteTextures);
    khash_t(tex) *list = state.texture.list;
    if (list) {
        khint_t k;
        gltexture_t *tex;
        for (int i = 0; i < n; i++) {
            GLuint t = textures[i];
            k = kh_get(tex, list, t);
            if (k != kh_end(list)) {
                tex = kh_value(list, k);
                if (tex == state.texture.bound)
                    state.texture.bound = NULL;
                free(tex);
                kh_del(tex, list, k);
            }
        }
    }
    LOAD_GLES(glDeleteTextures);
    gles_glDeleteTextures(n, textures);
}

GLboolean glAreTexturesResident(GLsizei n, const GLuint *textures, GLboolean *residences) {
    return true;
}
