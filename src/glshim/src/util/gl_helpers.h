#include <GL/gl.h>
#include <stdbool.h>
#include <stdio.h>

#ifndef GL_HELPERS_H
#define GL_HELPERS_H

#define GL_TYPE_CASE(name, var, magic, type, code) \
    case magic: {                                  \
        type *name = (type *)var;                  \
        code                                       \
        break;                                     \
    }

#define GL_TYPE_SWITCH(name, var, type, code, extra)               \
    switch (type) {                                                \
        GL_TYPE_CASE(name, var, GL_DOUBLE, GLdouble, code)         \
        GL_TYPE_CASE(name, var, GL_FLOAT, GLfloat, code)           \
        GL_TYPE_CASE(name, var, GL_INT, GLint, code)               \
        GL_TYPE_CASE(name, var, GL_SHORT, GLshort, code)           \
        GL_TYPE_CASE(name, var, GL_UNSIGNED_BYTE, GLubyte, code)   \
        GL_TYPE_CASE(name, var, GL_UNSIGNED_INT, GLuint, code)     \
        GL_TYPE_CASE(name, var, GL_UNSIGNED_SHORT, GLushort, code) \
        extra                                                      \
    }

static const GLsizei gl_sizeof(GLenum type) {
    // types
    switch (type) {
        case GL_DOUBLE:
            return 8;
        case GL_FLOAT:
        case GL_INT:
        case GL_UNSIGNED_INT:
        case GL_UNSIGNED_INT_10_10_10_2:
        case GL_UNSIGNED_INT_2_10_10_10_REV:
        case GL_UNSIGNED_INT_8_8_8_8:
        case GL_UNSIGNED_INT_8_8_8_8_REV:
        case GL_4_BYTES:
            return 4;
        case GL_3_BYTES:
            return 3;
        case GL_LUMINANCE_ALPHA:
        case GL_UNSIGNED_SHORT:
        case GL_UNSIGNED_SHORT_1_5_5_5_REV:
        case GL_UNSIGNED_SHORT_4_4_4_4:
        case GL_UNSIGNED_SHORT_4_4_4_4_REV:
        case GL_UNSIGNED_SHORT_5_5_5_1:
        case GL_UNSIGNED_SHORT_5_6_5:
        case GL_UNSIGNED_SHORT_5_6_5_REV:
        case GL_2_BYTES:
            return 2;
        case GL_LUMINANCE:
        case GL_UNSIGNED_BYTE:
        case GL_UNSIGNED_BYTE_2_3_3_REV:
        case GL_UNSIGNED_BYTE_3_3_2:
            return 1;
    }
    // formats
    printf("gl_sizeof(): Unknown data type 0x%x\n", type);
    return 0;
}

static const GLuint gl_max_value(GLenum type) {
    switch (type) {
        // float/double only make sense on tex/color
        // be careful about using this
        case GL_DOUBLE:
        case GL_FLOAT:
            return 1;
        case GL_BYTE:           return 127;
        case GL_UNSIGNED_BYTE:  return 255;
        case GL_SHORT:          return 32767;
        case GL_UNSIGNED_SHORT: return 65535;
        case GL_INT:            return 2147483647;
        case GL_UNSIGNED_INT:   return 4294967295;
    }
    printf("gl_max_value(): Unknown GL type 0x%x\n", type);
    return 0;
}

static const bool gl_is_type_packed(GLenum type) {
    switch (type) {
        case GL_4_BYTES:
        case GL_UNSIGNED_BYTE_2_3_3_REV:
        case GL_UNSIGNED_BYTE_3_3_2:
        case GL_UNSIGNED_INT_10_10_10_2:
        case GL_UNSIGNED_INT_2_10_10_10_REV:
        case GL_UNSIGNED_INT_8_8_8_8:
        case GL_UNSIGNED_INT_8_8_8_8_REV:
        case GL_UNSIGNED_SHORT_1_5_5_5_REV:
        case GL_UNSIGNED_SHORT_4_4_4_4:
        case GL_UNSIGNED_SHORT_4_4_4_4_REV:
        case GL_UNSIGNED_SHORT_5_5_5_1:
        case GL_UNSIGNED_SHORT_5_6_5:
        case GL_UNSIGNED_SHORT_5_6_5_REV:
            return true;
    }
    return false;
}

static const GLsizei gl_pixel_sizeof(GLenum format, GLenum type) {
    GLsizei width = 0;
    switch (format) {
        case GL_ALPHA:
        case GL_LUMINANCE:
        case GL_RED:
            width = 1;
            break;
        case GL_LUMINANCE_ALPHA:
        case GL_RG:
            width = 2;
            break;
        case GL_RGB:
        case GL_BGR:
            width = 3;
            break;
        case GL_RGBA:
        case GL_BGRA:
            width = 4;
            break;
        default:
            printf("gl_pixel_sizeof(): Unknown format %x\n", format);
            return 0;
    }

    if (gl_is_type_packed(type))
        width = 1;

    return width * gl_sizeof(type);
}

static inline const bool gl_valid_vertex_type(GLenum type) {
    switch (type) {
        case GL_BYTE:
        case GL_FIXED:
        case GL_FLOAT:
        case GL_SHORT:
            return true;
        default:
            return false;
    }
}

static inline const bool gl_valid_mode(GLenum mode) {
    switch (mode) {
        case GL_POINTS:
        case GL_LINES:
        case GL_LINE_STRIP:
        case GL_LINE_LOOP:
        case GL_TRIANGLES:
        case GL_TRIANGLE_STRIP:
        case GL_TRIANGLE_FAN:
        case GL_QUADS:
        case GL_QUAD_STRIP:
        case GL_POLYGON:
            return true;
        default:
            return false;
    }
}

static inline const size_t gl_getv_length(GLenum name) {
    switch (name) {
        // GL_BOOL
        case GL_ALPHA_BITS:
        case GL_ALPHA_TEST:
        case GL_BLEND:
        case GL_CLIP_PLANE0:
        case GL_COLOR_ARRAY:
        case GL_COLOR_LOGIC_OP:
        case GL_COLOR_MATERIAL:
        case GL_CULL_FACE:
        case GL_CURRENT_RASTER_POSITION_VALID:
        case GL_DEPTH_TEST:
        case GL_DEPTH_WRITEMASK:
        case GL_FOG:
        case GL_LIGHT0:
        case GL_LIGHTING:
        case GL_LIGHT_MODEL_TWO_SIDE:
        case GL_LINE_SMOOTH:
        case GL_MULTISAMPLE:
        case GL_NORMALIZE:
        case GL_NORMAL_ARRAY:
        // case GL_POINT_SIZE_ARRAY_OES:
        case GL_POINT_SMOOTH:
        // case GL_POINT_SPRITE_OES:
        case GL_POLYGON_OFFSET_FILL:
        case GL_RESCALE_NORMAL:
        case GL_SAMPLE_ALPHA_TO_COVERAGE:
        case GL_SAMPLE_ALPHA_TO_ONE:
        case GL_SAMPLE_COVERAGE:
        case GL_SAMPLE_COVERAGE_INVERT:
        case GL_SCISSOR_TEST:
        case GL_STENCIL_TEST:
        case GL_TEXTURE_GEN_Q:
        case GL_TEXTURE_GEN_R:
        case GL_TEXTURE_GEN_S:
        case GL_TEXTURE_GEN_T:
            return 1;
        // GL_FLOAT
        case GL_MODELVIEW_MATRIX:
        case GL_PROJECTION_MATRIX:
        case GL_TEXTURE_MATRIX:
            return 16;
        case GL_COLOR_CLEAR_VALUE:
        case GL_COLOR_WRITEMASK:
        case GL_CURRENT_COLOR:
        case GL_CURRENT_RASTER_COLOR:
        case GL_CURRENT_RASTER_POSITION:
        case GL_CURRENT_TEXTURE_COORDS:
        case GL_FOG_COLOR:
        case GL_LIGHT_MODEL_AMBIENT:
        case GL_SAMPLE_COVERAGE_VALUE:
        case GL_SCISSOR_BOX:
        case GL_VIEWPORT:
            return 4;
        case GL_CURRENT_NORMAL:
        case GL_POINT_DISTANCE_ATTENUATION:
            return 3;
        case GL_ALIASED_LINE_WIDTH_RANGE:
        case GL_ALIASED_POINT_SIZE_RANGE:
        case GL_DEPTH_RANGE:
        case GL_MAX_VIEWPORT_DIMS:
        case GL_SMOOTH_LINE_WIDTH_RANGE:
        case GL_SMOOTH_POINT_SIZE_RANGE:
            return 2;
        case GL_ALPHA_TEST_REF:
        case GL_DEPTH_CLEAR_VALUE:
        case GL_FOG_DENSITY:
        case GL_FOG_END:
        case GL_FOG_START:
        case GL_LINE_WIDTH:
        case GL_POINT_FADE_THRESHOLD_SIZE:
        case GL_POINT_SIZE_MAX:
        case GL_POINT_SIZE_MIN:
        case GL_POLYGON_OFFSET_FACTOR:
        case GL_POLYGON_OFFSET_UNITS:
        case GL_STENCIL_BACK_REF:
        case GL_STENCIL_REF:
            return 1;
        // GL_INT
        case GL_ACTIVE_TEXTURE:
        case GL_ALPHA_TEST_FUNC:
        case GL_ARRAY_BUFFER_BINDING:
        case GL_ATTRIB_STACK_DEPTH:
        case GL_AUX_BUFFERS:
        case GL_BLEND_DST:
        case GL_BLEND_SRC:
        case GL_BLUE_BITS:
        case GL_CLIENT_ACTIVE_TEXTURE:
        case GL_CLIENT_ATTRIB_STACK_DEPTH:
        case GL_COLOR_ARRAY_BUFFER_BINDING:
        case GL_COLOR_ARRAY_SIZE:
        case GL_COLOR_ARRAY_STRIDE:
        case GL_COLOR_ARRAY_TYPE:
        case GL_CULL_FACE_MODE:
        case GL_DEPTH_BITS:
        case GL_DEPTH_FUNC:
        case GL_ELEMENT_ARRAY_BUFFER_BINDING:
        case GL_FOG_HINT:
        case GL_FOG_MODE:
        case GL_FRONT_FACE:
        case GL_GREEN_BITS:
        // case GL_IMPLEMENTATION_COLOR_READ_FORMAT_OES:
        // case GL_IMPLEMENTATION_COLOR_READ_TYPE_OES:
        case GL_LINE_SMOOTH_HINT:
        case GL_LOGIC_OP_MODE:
        case GL_MATRIX_MODE:
        case GL_MAX_ATTRIB_STACK_DEPTH:
        case GL_MAX_CLIENT_ATTRIB_STACK_DEPTH:
        case GL_MAX_CLIP_PLANES:
        case GL_MAX_ELEMENTS_INDICES:
        case GL_MAX_LIGHTS:
        case GL_MAX_MODELVIEW_STACK_DEPTH:
        case GL_MAX_NAME_STACK_DEPTH:
        case GL_MAX_PROJECTION_STACK_DEPTH:
        case GL_MAX_TEXTURE_SIZE:
        case GL_MAX_TEXTURE_STACK_DEPTH:
        case GL_MAX_TEXTURE_UNITS:
        case GL_MODELVIEW_STACK_DEPTH:
        case GL_NAME_STACK_DEPTH:
        case GL_NORMAL_ARRAY_BUFFER_BINDING:
        case GL_NORMAL_ARRAY_STRIDE:
        case GL_NORMAL_ARRAY_TYPE:
        case GL_NUM_COMPRESSED_TEXTURE_FORMATS:
        case GL_PACK_ALIGNMENT:
        case GL_PERSPECTIVE_CORRECTION_HINT:
        case GL_POINT_SIZE:
        // case GL_POINT_SIZE_ARRAY_BUFFER_BINDING_OES:
        // case GL_POINT_SIZE_ARRAY_STRIDE_OES:
        // case GL_POINT_SIZE_ARRAY_TYPE_OES:
        case GL_POINT_SMOOTH_HINT:
        case GL_PROJECTION_STACK_DEPTH:
        case GL_RED_BITS:
        case GL_SAMPLES:
        case GL_SAMPLE_BUFFERS:
        case GL_SHADE_MODEL:
        case GL_STENCIL_BACK_FAIL:
        case GL_STENCIL_BACK_FUNC:
        case GL_STENCIL_BACK_PASS_DEPTH_FAIL:
        case GL_STENCIL_BACK_PASS_DEPTH_PASS:
        case GL_STENCIL_BACK_VALUE_MASK:
        case GL_STENCIL_BACK_WRITEMASK:
        case GL_STENCIL_BITS:
        case GL_STENCIL_CLEAR_VALUE:
        case GL_STENCIL_FAIL:
        case GL_STENCIL_FUNC:
        case GL_STENCIL_PASS_DEPTH_FAIL:
        case GL_STENCIL_PASS_DEPTH_PASS:
        case GL_STENCIL_VALUE_MASK:
        case GL_STENCIL_WRITEMASK:
        case GL_SUBPIXEL_BITS:
        case GL_TEXTURE_BINDING_2D:
        case GL_TEXTURE_BINDING_CUBE_MAP:
        case GL_TEXTURE_STACK_DEPTH:
        case GL_UNPACK_ALIGNMENT:
            return 1;
        // should consider warning on default
        default:
            return 1;
    }
}

static inline const size_t gl_fogv_length(GLenum name) {
    switch (name) {
        case GL_FOG_MODE:
        case GL_FOG_DENSITY:
        case GL_FOG_START:
        case GL_FOG_END:
        case GL_FOG_INDEX:
        case GL_FOG_COORD_SRC:
            return 1;
        case GL_FOG_COLOR:
            return 4;
        default:
            fprintf(stderr, "libGL: warning: gl_fogv_length unknown pname 0x%x\n", name);
            return 1;
    }
}

#endif
