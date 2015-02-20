#include "gl.h"
#include <math.h>

extern void glTexGenfv(GLenum coord, GLenum pname, GLfloat *params);
extern void glTexGeni(GLenum coord, GLenum pname, GLint param);
extern void gen_tex_coords(GLfloat *verts, GLfloat **coords, GLint count);
