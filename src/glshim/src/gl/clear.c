#include "error.h"
#include "loader.h"
#include "remote.h"

void glClear(GLbitfield mask) {
    ERROR_IN_BLOCK();
    PUSH_IF_COMPILING(glClear);
    PROXY_GLES(glClear);
}

void glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) {
    ERROR_IN_BLOCK();
    PUSH_IF_COMPILING(glClearColor);
    PROXY_GLES(glClearColor);
}

void glClearDepthf(GLclampf depth) {
    ERROR_IN_BLOCK();
    PUSH_IF_COMPILING(glClearDepthf);
    PROXY_GLES(glClearDepthf);
}

void glClearStencil(GLint s) {
    ERROR_IN_BLOCK();
    PUSH_IF_COMPILING(glClearStencil);
    PROXY_GLES(glClearStencil);
}
