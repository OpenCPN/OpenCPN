#ifndef GL_TYPES_H
#define GL_TYPES_H

#include <GL/gl.h>
#include <stdbool.h>
#include <stdint.h>

#include "../config.h"
#include "extypes.h"
#include "khash.h"
#include "tack.h"
#include "wrap/types.h"
#include "../util/mat4.h"
#include "ring.h"

// block.h
typedef struct {
    uint32_t len, count, cap;
    GLenum mode;
    struct {
        GLfloat tex[MAX_TEX][2];
    } last;

    // TODO: dynamic type support?
    /*
    struct {
        GLenum vert, normal, color, tex;
    } type;
    */

    GLfloat *vert;
    GLfloat *normal;
    GLfloat *color;
    GLfloat *tex[MAX_TEX];
    GLushort *indices;
    GLboolean q2t;

    struct {
        int tex[MAX_TEX], color, normal;
    } incomplete;

    GLboolean open;
    GLboolean artificial;
    void *solid;
} block_t;

typedef struct {
    int index;
    block_t *block;
} block_call_t;

// eval.h
typedef struct {
    GLenum type;
} map_state_t;

typedef struct {
    GLdouble _1, _2, n, d;
    GLint stride, order;
} mapcoordd_t;

typedef struct {
    GLdouble _1, _2, n, d;
    GLint stride, order;
} mapcoordf_t;

typedef struct {
    GLenum type;
    GLint dims, width;
    mapcoordd_t u, v;
    GLboolean free;
    const GLdouble *points;
} map_stated_t;

typedef struct {
    GLenum type;
    GLint dims, width;
    mapcoordf_t u, v;
    GLboolean free;
    const GLfloat *points;
} map_statef_t;

// list.h
typedef struct {
    bool open;
    tack_t calls;
} displaylist_t;

// texture.h
typedef struct {
    GLuint texture;
    GLenum target;
    GLsizei width;
    GLsizei height;
    GLsizei nwidth;
    GLsizei nheight;
    GLboolean uploaded;
} gltexture_t;

KHASH_MAP_INIT_INT(tex, gltexture_t *)

// state.h
typedef struct {
    GLboolean line_stipple,
              blend,
              color_array,
              normal_array,
              tex_coord_array[MAX_TEX],
              texgen_q[MAX_TEX],
              texgen_r[MAX_TEX],
              texgen_s[MAX_TEX],
              texgen_t[MAX_TEX],
              texture_2d[MAX_TEX],
              vertex_array;
} enable_state_t;

typedef struct {
    GLenum R, Q, S, T;
    GLfloat Rv[4], Qv[4], Sv[4], Tv[4];
} texgen_state_t;

typedef struct {
    GLuint pack_row_length,
           pack_skip_pixels,
           pack_skip_rows,
           unpack_row_length,
           unpack_skip_pixels,
           unpack_skip_rows;
    GLboolean pack_swap_bytes,
              pack_lsb_first,
              unpack_swap_bytes,
              unpack_lsb_first;
    // TODO: do we only need to worry about GL_TEXTURE_2D?
    GLboolean rect_arb[MAX_TEX];
    gltexture_t *bound[MAX_TEX];
    khash_t(tex) *list;
    // active textures
    GLuint active;
    GLuint client;
} texture_state_t;

typedef struct {
    GLint size;
    GLenum type;
    GLsizei stride;
    const GLvoid *pointer;
} pointer_state_t;

typedef struct {
    pointer_state_t vertex, color, normal, tex_coord[MAX_TEX];
} pointer_states_t;

typedef struct {
    GLfloat color[4];
    GLfloat normal[3];
    GLfloat tex[MAX_TEX][2];
} current_state_t;

typedef struct {
    displaylist_t *active;
    current_state_t current;

    GLuint base;
    GLuint name;
    GLenum mode;
    GLuint recursion;
} displaylist_state_t;

typedef struct {
    block_t *active;
    GLboolean locked;
} block_state_t;

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

// matrix structs
typedef struct {
    mat4 matrix;
    tack_t stack;
    bool init;
} matrix_state_t;

typedef struct {
    GLenum mode;
    matrix_state_t model, projection, texture[MAX_TEX], color;
} matrix_states_t;

typedef struct {
    GLboolean overflow;
    GLint count;
    GLsizei size;
    GLuint *buffer;
    tack_t names;
} select_state_t;

typedef struct {
    GLboolean overflow;
    GLenum type;
    GLfloat *buffer;
    GLint count, values;
    GLsizei size;
} feedback_state_t;

typedef struct {
    GLenum mode;
} render_state_t;

typedef struct {
    GLubyte *buf;
    struct {
        GLfloat x, y, z, w;
    } pos;
    GLfloat color[4];
    GLuint pixel;
    GLboolean valid;
} raster_state_t;

typedef struct {
    GLfloat x, y, width, height, nwidth, nheight;
} viewport_state_t;

typedef struct {
    tack_t attrib, client;
} stack_state_t;

// global state struct
typedef struct {
    displaylist_state_t list;
    tack_t lists;
    int64_t remote;
    ring_t *remote_ring;

    GLenum error;
    block_state_t block;
    current_state_t current;
    enable_state_t enable;
    feedback_state_t feedback;
    map_state_t *map_grid;
    map_states_t map1, map2;
    matrix_states_t matrix;
    pointer_states_t pointers;
    raster_state_t raster;
    render_state_t render;
    select_state_t select;
    stack_state_t stack;
    texgen_state_t texgen[MAX_TEX];
    texture_state_t texture;
    viewport_state_t viewport;
} glstate_t;

extern glstate_t state;
#define CURRENT (state.list.active ? &state.list.current : &state.current)

#endif
