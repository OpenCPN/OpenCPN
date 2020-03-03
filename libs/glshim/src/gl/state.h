#include "gl.h"

#ifndef GL_STATE_H
#define GL_STATE_H

#include "eval.h"
#include "texture.h"

typedef struct {
    GLboolean line_stipple,
              blend,
              color_array,
              normal_array,
              tex_coord_array,
              texgen_s,
              texgen_t,
              texture_2d,
              vertex_array;
} enable_state_t;


typedef struct {
    GLenum S;
    GLenum T;
    GLfloat Sv[4];
    GLfloat Tv[4];
} texgen_state_t;

typedef struct {
    GLuint unpack_row_length,
           unpack_skip_pixels,
           unpack_skip_rows;
    GLboolean unpack_lsb_first;
    // TODO: do we only need to worry about GL_TEXTURE_2D?
    GLboolean rect_arb;
    gltexture_t *bound;
    khash_t(tex) *list;
} texture_state_t;


typedef struct {
    GLint size;
    GLenum type;
    GLsizei stride;
    const GLvoid *pointer;
} pointer_state_t;

typedef struct {
    pointer_state_t vertex, color, normal, tex_coord;
} pointer_states_t;


typedef struct {
    renderlist_t *active;
    renderlist_t *first;
    GLboolean compiling;
    GLboolean locked;
    GLuint base;
    GLuint name;
    GLenum mode;

    GLuint count;
    GLuint cap;
} displaylist_state_t;


typedef struct {
    map_state_t *vertex3,
                *vertex4,
                *index,
                *color4,
                *normal,
                *texture1,
                *texture2,
                *texture3,
                *texture4;
} map_states_t;


typedef struct {
    displaylist_state_t list;
    enable_state_t enable;
    map_state_t *map_grid;
    map_states_t map1, map2;
    pointer_states_t pointers;
    renderlist_t **lists;
    texgen_state_t texgen;
    texture_state_t texture;
    GLfloat color[4];
} glstate_t;

#endif
