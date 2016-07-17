#ifndef GL_TEXTURE_H
#define GL_TEXTURE_H

#include <GL/gl.h>

void glTexImage2D(GLenum target, GLint level, GLint internalFormat,
                  GLsizei width, GLsizei height, GLint border,
                  GLenum format, GLenum type, const GLvoid *data);

void glTexImage1D(GLenum target, GLint level, GLint internalFormat,
                  GLsizei width, GLint border,
                  GLenum format, GLenum type, const GLvoid *data);

void glTexImage3D(GLenum target, GLint level, GLint internalFormat,
                  GLsizei width, GLsizei height, GLsizei depth,
                  GLint border, GLenum format, GLenum type, const GLvoid *data);

void glBindTexture(GLenum target, GLuint texture);
void glDeleteTextures(GLsizei n, const GLuint * textures);
void glTexParameteri(GLenum target, GLenum pname, GLint param);
GLboolean glAreTexturesResident(GLsizei n, const GLuint *textures, GLboolean *residences);

void tex_coord_rect_arb(GLfloat *tex, GLsizei len,
                        GLsizei width, GLsizei height);
void tex_coord_npot(GLfloat *tex, GLsizei len,
                    GLsizei width, GLsizei height,
                    GLsizei nwidth, GLsizei nheight);
int npot(int n);

static inline GLenum map_tex_target(GLenum target) {
    switch (target) {
        case GL_TEXTURE_1D:
        case GL_TEXTURE_3D:
        case GL_TEXTURE_RECTANGLE_ARB:
            target = GL_TEXTURE_2D;
            break;
        case GL_PROXY_TEXTURE_1D:
        case GL_PROXY_TEXTURE_3D:
            target = GL_PROXY_TEXTURE_2D;
            break;
    }
    return target;
}

#endif
