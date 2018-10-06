#include "gl.h"

#ifndef DISPLAY_LIST_H
#define DISPLAY_LIST_H

typedef struct {
    int face;
    int pname;
    GLfloat color[4];
    int count;
} rendermaterial_t;

KHASH_MAP_INIT_INT(material, rendermaterial_t *)

typedef struct _call_list_t {
    unsigned long len;
    unsigned long cap;
    packed_call_t **calls;
} call_list_t;

typedef struct _renderlist_t {
    unsigned long len;
    unsigned long cap;
    GLenum mode;
    GLfloat lastNormal[3];
    GLfloat lastTex[2];

    call_list_t calls;
    GLfloat *vert;
    GLfloat *normal;
    GLfloat *color;
    GLfloat *tex;
    GLushort *indices;
    GLboolean q2t;

    khash_t(material) *material;
    GLuint texture;
    struct _renderlist_t *prev;
    struct _renderlist_t *next;
    GLboolean open;
} renderlist_t;

#define DEFAULT_CALL_LIST_CAPACITY 20
#define DEFAULT_RENDER_LIST_CAPACITY 20

extern renderlist_t *alloc_renderlist();
extern renderlist_t *extend_renderlist(renderlist_t *list);
extern void free_renderlist(renderlist_t *list);
extern void draw_renderlist(renderlist_t *list);
extern void q2t_renderlist(renderlist_t *list);
extern void end_renderlist(renderlist_t *list);

extern void rlBindTexture(renderlist_t *list, GLuint texture);
extern void rlColor4f(renderlist_t *list, GLfloat r, GLfloat g, GLfloat b, GLfloat a);
extern void rlMaterialfv(renderlist_t *list, GLenum face, GLenum pname, const GLfloat * params);
extern void rlNormal3f(renderlist_t *list, GLfloat x, GLfloat y, GLfloat z);
extern void rlPushCall(renderlist_t *list, packed_call_t *data);
extern void rlTexCoord2f(renderlist_t *list, GLfloat s, GLfloat t);
extern void rlVertex3f(renderlist_t *list, GLfloat x, GLfloat y, GLfloat z);

#endif
