#include "gl.h"

extern void glLineStipple(GLuint factor, GLushort pattern);
extern GLfloat *gen_stipple_tex_coords(GLfloat *vert, int length);
extern void bind_stipple_tex();
