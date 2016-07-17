#include <GL/gl.h>

#include "error.h"
#include "gl_helpers.h"
#include "gl_str.h"
#include "loader.h"
#include "matrix.h"
#include "remote.h"

void gl_set_error(GLenum error) {
    // call upstream glGetError to clear the driver's error flag
    if (state.remote) {
        forward_glGetError();
    } else {
        LOAD_GLES(glGetError);
        gles_glGetError();
    }
    state.error = error;
}

// calls upstream glGetError and saves the flag for the next caller
GLenum gl_get_error() {
    if (state.remote) {
        state.error = forward_glGetError();
    } else {
        LOAD_GLES(glGetError);
        state.error = gles_glGetError();
    }
    return state.error;
}

GLenum glGetError() {
    if (state.block.active) {
        return GL_INVALID_OPERATION;
    }
    GLenum error;
    if (state.remote) {
        error = forward_glGetError();
    } else {
        LOAD_GLES(glGetError);
        error = gles_glGetError();
    }
    if (error == GL_NO_ERROR) {
        error = state.error;
    }
    state.error = GL_NO_ERROR;
    return error;
}

// config functions
const GLubyte *glGetString(GLenum name) {
    FORWARD_IF_REMOTE(glGetString);
    LOAD_GLES(glGetString);
    if (state.block.active) {
        gl_set_error(GL_INVALID_OPERATION);
        return NULL;
    }
    switch (name) {
        case GL_VERSION:
#ifdef USE_ES2
            return (GLubyte *)"4.3 glshim wrapper";
#else
            return (GLubyte *)"1.4 glshim wrapper";
#endif
	    #if 0
        case GL_EXTENSIONS:
            return (const GLubyte *)(char *){
#ifndef USE_ES2
                // "GL_ARB_vertex_buffer_object "
                "GL_ARB_multitexture "
                "GL_ARB_texture_cube_map "
                "GL_EXT_secondary_color "
                "GL_EXT_texture_env_combine "
                "GL_EXT_texture_env_dot3 "

                // blending extensions
                "GL_EXT_blend_color "
                "GL_EXT_blend_equation_separate "
                "GL_EXT_blend_func_separate "
                "GL_EXT_blend_logic_op "
                "GL_EXT_blend_subtract "
#else
                "GL_ARB_vertex_shader "
                "GL_ARB_fragment_shader "
                "GL_ARB_vertex_buffer_object "
                "GL_EXT_framebuffer_object "
#endif
            };
	    #endif
        default:
            return gles_glGetString(name);
    }
}

void gl_get(GLenum pname, GLenum type, GLvoid *params) {
    if (state.remote) {
        remote_gl_get(pname, type, params);
        return;
    }
    LOAD_GLES(glGetBooleanv);
    LOAD_GLES(glGetFloatv);
    LOAD_GLES(glGetIntegerv);
    ERROR_IN_BLOCK();

    int width = gl_getv_length(pname);
    switch (pname) {
        // GL_BOOL
        case GL_CURRENT_RASTER_POSITION_VALID:
        case GL_PACK_LSB_FIRST:
        case GL_PACK_SWAP_BYTES:
        case GL_TEXTURE_GEN_Q:
        case GL_TEXTURE_GEN_R:
        case GL_TEXTURE_GEN_S:
        case GL_TEXTURE_GEN_T:
        case GL_UNPACK_LSB_FIRST:
        case GL_UNPACK_SWAP_BYTES:
        {
            enable_state_t *enable = &state.enable;
            GLboolean tmp[4];
            GLboolean *out = tmp;
            if (type == GL_BOOL) {
                out = params;
            }
            switch (pname) {
                case GL_CURRENT_RASTER_POSITION_VALID:
                    *out = state.raster.valid;
                    break;
                case GL_TEXTURE_GEN_Q:
                    *out = enable->texgen_q[state.texture.active];
                    break;
                case GL_TEXTURE_GEN_R:
                    *out = enable->texgen_r[state.texture.active];
                    break;
                case GL_TEXTURE_GEN_S:
                    *out = enable->texgen_s[state.texture.active];
                    break;
                case GL_TEXTURE_GEN_T:
                    *out = enable->texgen_t[state.texture.active];
                    break;
                case GL_PACK_LSB_FIRST:
                    *out = state.texture.pack_lsb_first;
                    break;
                case GL_PACK_SWAP_BYTES:
                    *out = state.texture.pack_swap_bytes;
                    break;
                case GL_UNPACK_LSB_FIRST:
                    *out = state.texture.unpack_lsb_first;
                    break;
                case GL_UNPACK_SWAP_BYTES:
                    *out = state.texture.unpack_swap_bytes;
                    break;
            }
            if (type != GL_BOOL) {
                for (int i = 0; i < width; i++) {
                    if (type == GL_INT) {
                        GLint *ret = params;
                        ret[i] = out[i];
                    } else if (type == GL_FLOAT) {
                        GLfloat *ret = params;
                        ret[i] = out[i];
                    }
                }
            }
            break;
        }
        // GL_FLOAT
        case GL_CURRENT_COLOR:
        case GL_CURRENT_NORMAL:
        case GL_CURRENT_RASTER_COLOR:
        case GL_CURRENT_RASTER_POSITION:
        case GL_CURRENT_TEXTURE_COORDS:
        case GL_MODELVIEW_MATRIX:
        case GL_PROJECTION_MATRIX:
        case GL_TEXTURE_MATRIX:
        {
            bool scale = false;
            GLfloat tmp[4];
            GLfloat *out = tmp;
            if (type == GL_FLOAT) {
                out = params;
            }
            switch (pname) {
                case GL_CURRENT_COLOR:
                    memcpy(out, &CURRENT->color, sizeof(GLfloat) * 4);
                    break;
                case GL_CURRENT_NORMAL:
                    memcpy(out, &CURRENT->normal, sizeof(GLfloat) * 3);
                    break;
                case GL_CURRENT_RASTER_COLOR:
                    memcpy(out, &state.raster.color, sizeof(GLfloat) * 4);
                    break;
                case GL_CURRENT_RASTER_POSITION:
                    memcpy(out, &state.raster.pos, sizeof(GLfloat) * 4);
                    break;
                case GL_CURRENT_TEXTURE_COORDS:
                    memcpy(out, &CURRENT->tex, sizeof(GLfloat) * 2);
                    // TODO: need to update this when I track 4d texture coordinates
                    out[3] = 0;
                    out[4] = 0;
                    break;
                case GL_MODELVIEW_MATRIX:
                    gl_get_matrix(GL_MODELVIEW, out);
                    break;
                case GL_PROJECTION_MATRIX:
                    gl_get_matrix(GL_PROJECTION, out);
                    break;
                case GL_TEXTURE_MATRIX:
                    gl_get_matrix(GL_TEXTURE, out);
                    break;
            }
            if (type != GL_FLOAT) {
                for (int i = 0; i < width; i++) {
                    if (type == GL_INT) {
                        GLint *ret = params;
                        if (scale) {
                            ret[i] = out[i] * gl_max_value(type);
                        } else {
                            ret[i] = out[i];
                        }
                    } else if (type == GL_BOOL) {
                        GLboolean *ret = params;
                        ret[i] = !! out[i];
                    }
                }
            }
            break;
        }
        // GL_INT
        case GL_ATTRIB_STACK_DEPTH:
        case GL_AUX_BUFFERS:
        case GL_CLIENT_ATTRIB_STACK_DEPTH:
        case GL_MAJOR_VERSION:
        case GL_MAX_ATTRIB_STACK_DEPTH:
        case GL_MAX_CLIENT_ATTRIB_STACK_DEPTH:
        case GL_MAX_ELEMENTS_INDICES:
        case GL_MAX_MODELVIEW_STACK_DEPTH:
        case GL_MAX_NAME_STACK_DEPTH:
        case GL_MAX_PROJECTION_STACK_DEPTH:
        case GL_MAX_TEXTURE_STACK_DEPTH:
        case GL_MINOR_VERSION:
        case GL_MODELVIEW_STACK_DEPTH:
        case GL_NAME_STACK_DEPTH:
        case GL_PACK_ROW_LENGTH:
        case GL_PACK_SKIP_PIXELS:
        case GL_PACK_SKIP_ROWS:
        case GL_PROJECTION_STACK_DEPTH:
        case GL_TEXTURE_STACK_DEPTH:
        case GL_UNPACK_ROW_LENGTH:
        case GL_UNPACK_SKIP_PIXELS:
        case GL_UNPACK_SKIP_ROWS:
        {
            GLint tmp[4];
            GLint *out = tmp;
            if (type == GL_INT) {
                out = params;
            }
            switch (pname) {
                case GL_ATTRIB_STACK_DEPTH:
                    *out = tack_len(&state.stack.attrib);
                    break;
                case GL_AUX_BUFFERS:
                    *out = 0;
                    break;
                case GL_CLIENT_ATTRIB_STACK_DEPTH:
                    *out = tack_len(&state.stack.client);
                    break;
                case GL_MAX_LIST_NESTING:
                    *out = 64;
                    break;
                case GL_MAJOR_VERSION:
                    *out = 1;
                    break;
                case GL_MINOR_VERSION:
                    *out = 4;
                    break;
                case GL_MAX_ATTRIB_STACK_DEPTH:
                case GL_MAX_CLIENT_ATTRIB_STACK_DEPTH:
                case GL_MAX_ELEMENTS_INDICES:
                case GL_MAX_MODELVIEW_STACK_DEPTH:
                case GL_MAX_NAME_STACK_DEPTH:
                case GL_MAX_PROJECTION_STACK_DEPTH:
                case GL_MAX_TEXTURE_STACK_DEPTH:
                    // NOTE: GL_MAX_ELEMENTS_INDICES is *actually* 65535, the others in this group are arbitrary
                    *out = 65535;
                    break;
                case GL_MODELVIEW_STACK_DEPTH:
                    *out = tack_len(&state.matrix.model.stack);
                    break;
                case GL_NAME_STACK_DEPTH:
                    *out = tack_len(&state.select.names);
                    break;
                case GL_PROJECTION_STACK_DEPTH:
                    *out = tack_len(&state.matrix.projection.stack);
                    break;
                case GL_TEXTURE_STACK_DEPTH:
                    *out = tack_len(&state.matrix.texture[state.texture.active].stack);
                    break;
                // texture stuff
                case GL_PACK_ROW_LENGTH:
                    *out = state.texture.pack_row_length;
                    break;
                case GL_PACK_SKIP_PIXELS:
                    *out = state.texture.pack_skip_pixels;
                    break;
                case GL_PACK_SKIP_ROWS:
                    *out = state.texture.pack_skip_rows;
                    break;
                case GL_UNPACK_ROW_LENGTH:
                    *out = state.texture.unpack_row_length;
                    break;
                case GL_UNPACK_SKIP_PIXELS:
                    *out = state.texture.unpack_skip_pixels;
                    break;
                case GL_UNPACK_SKIP_ROWS:
                    *out = state.texture.unpack_skip_rows;
                    break;
            }
            if (type != GL_INT) {
                for (int i = 0; i < width; i++) {
                    if (type == GL_FLOAT) {
                        GLfloat *ret = params;
                        ret[i] = out[i];
                    } else if (type == GL_BOOL) {
                        GLboolean *ret = params;
                        ret[i] = !! out[i];
                    }
                }
            }
            break;
        }
        default:
        {
            GLenum saved = glGetError();
            switch (type) {
                case GL_BOOL:
                    gles_glGetBooleanv(pname, params);
                    break;
                case GL_FLOAT:
                    gles_glGetFloatv(pname, params);
                    break;
                case GL_INT:
                    gles_glGetIntegerv(pname, params);
                    break;
            }
            GLenum error = gl_get_error();
            if (error == GL_INVALID_ENUM) {
                fprintf(stderr, "libGL: GL_INVALID_ENUM when calling glGet<%s>(%s)\n", gl_str(type), gl_str(pname));
                GL_TYPE_SWITCH(ret, params, type, *ret = 0;,);
            }
            gl_set_error(error ? error : saved);
            break;
         }
    }
}

void glGetBooleanv(GLenum pname, GLboolean *params) {
    gl_get(pname, GL_BOOL, params);
}

void glGetFloatv(GLenum pname, GLfloat *params) {
    gl_get(pname, GL_FLOAT, params);
}

void glGetIntegerv(GLenum pname, GLint *params) {
    gl_get(pname, GL_INT, params);
}

/*
void glGetFixedv(GLenum pname, GLfloat *params) {
    // meh
}
*/
