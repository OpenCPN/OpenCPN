#include <GL/gl.h>

GLenum gl_get_error();
void gl_get(GLenum pname, GLenum type, GLvoid *params);
void gl_set_error(GLenum error);
