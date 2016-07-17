#include <dlfcn.h>
#include <inttypes.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "types.h"

#ifdef __linux__
#include <linux/limits.h>
#endif

#ifndef PATH_MAX
#define PATH_MAX 254
#endif

#ifdef __ARM_NEON__
#include <arm_neon.h>
#endif

#ifndef GL_H
#define GL_H

#include "../config.h"
#include "wrap/glpack.h"

#include "gl_helpers.h"
#include "defines.h"

#endif
