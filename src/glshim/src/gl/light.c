#include "error.h"
#include "gl_str.h"
#include "light.h"
#include "loader.h"
#include "matrix.h"
#include "remote.h"

#ifndef USE_ES2
void glLightModelf(GLenum pname, GLfloat param) {
    ERROR_IN_BLOCK();
    PUSH_IF_COMPILING(glLightModelf);
    LOAD_GLES(glLightModelf);
    switch (pname) {
        case GL_LIGHT_MODEL_AMBIENT:
        case GL_LIGHT_MODEL_TWO_SIDE:
            gles_glLightModelf(pname, param);
        default:
            printf("stubbed glLightModelf(%s, %.2f)\n", gl_str(pname), param);
            break;
    }
}

void glMaterialfv(GLenum face, GLenum pname, const GLfloat *params) {
    PUSH_IF_COMPILING(glMaterialfv);
    LOAD_GLES(glMaterialfv);
    gles_glMaterialfv(GL_FRONT_AND_BACK, pname, params);
}

#ifdef LOCAL_MATRIX
void glLightfv(GLenum light, GLenum pname, const GLfloat *params) {
    ERROR_IN_BLOCK();
    PUSH_IF_COMPILING(glLightfv);
    LOAD_GLES(glLightfv);
    GLfloat tmp[4];
    switch (pname) {
        case GL_POSITION:
            gl_transform_light(tmp, params);
            params = tmp;
        default:
            gles_glLightfv(light, pname, params);
            break;
    }
}
#endif
#endif
