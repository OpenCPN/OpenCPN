#include "get.h"

#define ERROR(pname) do { gl_set_error(pname); return; } while (0)
#define ERROR_IN_BLOCK() if (state.block.active) { ERROR(GL_INVALID_OPERATION); }
