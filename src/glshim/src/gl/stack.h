#include "gl.h"
#include "state.h"

#ifndef GL_STACK_H
#define GL_STACK_H

#define STACK_SIZE 16

typedef struct {
    GLbitfield mask;

    // GL_COLOR_BUFFER_BIT
    GLboolean alpha_test;
    GLint alpha_test_func;
    GLclampf alpha_test_ref;

    GLboolean blend;
    GLint blend_src_func;
    GLint blend_dst_func;

    GLboolean dither;

    GLboolean color_logic_op;
    GLint logic_op;

    GLfloat clear_color[4];
    GLfloat color_mask[4];

    // GL_CURRENT_BIT
    GLfloat color[4];
    GLfloat normal[4];
    GLfloat tex[4];

    // TODO: can only fill this via raster.c
    GLfloat raster_pos[3];
    GLboolean raster_valid;

    // GL_DEPTH_BUFFER_BIT
    GLboolean depth_test;
    GLint depth_func;
    GLfloat clear_depth;
    GLint depth_mask;

    // GL_ENABLE_BIT
    GLboolean cull_face;
    GLboolean normalize;
    GLboolean polygon_offset_fill;
    GLboolean stencil_test;
    GLboolean texture_2d;

    // GL_FOG_BIT
    GLboolean fog;
    GLfloat fog_color[4];
    GLfloat fog_density;
    GLfloat fog_start;
    GLfloat fog_end;
    GLint fog_mode;

    // GL_HINT_BIT
    GLint perspective_hint;
    GLint point_smooth_hint;
    GLint line_smooth_hint;
    GLint fog_hint;
    GLint mipmap_hint;

    // GL_LIGHTING_BIT
    GLboolean lighting;
    GLboolean *lights_enabled;
    GLfloat *lights;
    GLint light_model_ambient[4];
    GLint light_model_two_side;
    GLint shade_model;

    // GL_LINE_BIT
    GLboolean line_smooth;
    GLboolean line_stipple; // TODO: needs to be hooked locally?
    GLfloat line_width;

    // GL_LIST_BIT
    GLint list_base;

    // GL_MULTISAMPLE_BIT
    GLboolean multisample;
    GLboolean sample_alpha_to_coverage;
    GLboolean sample_alpha_to_one;
    GLboolean sample_coverage;

    // GL_POINT_BIT
    GLboolean point_smooth;
    GLfloat point_size;

    // TODO: GL_POLYGON_BIT
    // TODO: GL_POLYGON_STIPPLE_BIT

    // GL_SCISSOR_BIT
    GLboolean scissor_test;
    GLfloat scissor_box[4];

    // TODO: GL_STENCIL_BUFFER_BIT

    // GL_TEXTURE_BIT
    GLint texture;

    // TODO: GL_TRANSFORM_BIT
    // TODO: GL_VIEWPORT_BIT

    // dynamically-sized shenanigans
    GLboolean *clip_planes_enabled;
    GLfloat *clip_planes;

    // misc
    unsigned int len;
    unsigned int cap;
} glstack_t;

typedef struct {
    GLbitfield mask;

    // GL_CLIENT_PIXEL_STORE_BIT
    GLint pack_align;
    GLint unpack_align;
    GLuint unpack_row_length;
    GLuint unpack_skip_pixels;
    GLuint unpack_skip_rows;

    // GL_CLIENT_VERTEX_ARRAY_BIT
    GLboolean vert_enable;
    GLboolean color_enable;
    GLboolean tex_enable;
    GLboolean normal_enable;
    pointer_state_t verts;
    pointer_state_t color;
    pointer_state_t normal;
    pointer_state_t tex;

    unsigned int len;
    unsigned int cap;
} glclientstack_t;

void glPushClientAttrib(GLbitfield mask);
void glPopClientAttrib();
void glPushAttrib(GLbitfield mask);
void glPopAttrib();

#endif
