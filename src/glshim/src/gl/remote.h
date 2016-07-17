#ifndef GL_REMOTE_H
#define GL_REMOTE_H
#include <stdlib.h>
#include "block.h"
#include "wrap/types.h"

#define REMOTE_BLOCK_DRAW -1
#define REMOTE_GL_GET -2

int remote_serve(char *name);
int remote_spawn(const char *path);
void remote_write_block(ring_t *ring, block_t *block);
block_t *remote_read_block(ring_t *ring, packed_call_t *call);
void *remote_dma(size_t size);
void remote_dma_send(packed_call_t *call, void *ret_v);

void remote_block_draw(block_t *block);
void remote_gl_get(GLenum pname, GLenum type, GLvoid *params);

void remote_glEnable(GLenum cap);
void remote_glDisable(GLenum cap);
void remote_glEnableClientState(GLenum cap);
void remote_glDisableClientState(GLenum cap);
#endif
