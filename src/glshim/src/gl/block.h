#ifndef BLOCK_H
#define BLOCK_H

#include "types.h"

#define DEFAULT_BLOCK_CAPACITY 16
#define RENDER_BLOCK_INDEX -1

extern block_t *bl_new(GLenum mode);
extern void bl_free(block_t *block);
extern void bl_draw(block_t *block);
extern void bl_q2t(block_t *block);
extern void bl_end(block_t *block);

extern void bl_vertex3f(block_t *block, GLfloat x, GLfloat y, GLfloat z);
extern void bl_track_color(block_t *block);
extern void bl_track_normal(block_t *block);
extern void bl_track_tex(block_t *block, GLenum target);
extern void bl_pollute(block_t *block);

extern void bl_push_call(block_t *block, packed_call_t *data);

#endif
