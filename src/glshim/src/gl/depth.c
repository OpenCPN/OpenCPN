#include "error.h"
#include "loader.h"
#include "remote.h"

void glDepthFunc(GLenum func) {
    ERROR_IN_BLOCK();
    PUSH_IF_COMPILING(glDepthFunc);
    PROXY_GLES(glDepthFunc);
}

void glDepthMask(GLboolean flag) {
    ERROR_IN_BLOCK();
    PUSH_IF_COMPILING(glDepthMask);
    PROXY_GLES(glDepthMask);
}

void glDepthRangef(GLclampf near, GLclampf far) {
    ERROR_IN_BLOCK();
    PUSH_IF_COMPILING(glDepthRangef);
    PROXY_GLES(glDepthRangef);
}
