#include <X11/Xlib.h>

#include "../get.h"
#include "../list.h"
#include "../remote.h"
#include "./glpack.h"
#include "./remote.h"
#include "gl_helpers.h"

void remote_local_pre(ring_t *ring, packed_call_t *call) {
    switch (call->index) {
        case glXCreateContext_INDEX:
        {
            glXCreateContext_PACKED *n = (glXCreateContext_PACKED *)call;
            ring_write(ring, n->args.vis, sizeof(XVisualInfo));
            if (n->args.shareList) {
                ring_write(ring, n->args.shareList, sizeof(GLXContext));
            }
            break;
        }
        case glXMakeCurrent_INDEX:
            // if the window create hasn't flushed yet, we can't init on the remote
            XFlush(((glXMakeCurrent_PACKED *)call)->args.dpy);
            break;
        case glDeleteTextures_INDEX:
        {
            glDeleteTextures_PACKED *n = (glDeleteTextures_PACKED *)call;
            ring_write(ring, n->args.textures, n->args.n * sizeof(GLuint));
            break;
        }
        case glTexImage2D_INDEX:
        {
            glTexImage2D_PACKED *n = (glTexImage2D_PACKED *)call;
            if (n->args.pixels) {
                size_t size = n->args.width * n->args.height * gl_pixel_sizeof(n->args.format, n->args.type);
                ring_write(ring, n->args.pixels, size);
            }
            break;
        }
        case glTexSubImage2D_INDEX:
        {
            glTexSubImage2D_PACKED *n = (glTexSubImage2D_PACKED *)call;
            if (n->args.pixels) {
                size_t size = n->args.width * n->args.height * gl_pixel_sizeof(n->args.format, n->args.type);
                ring_write(ring, n->args.pixels, size);
            }
            break;
        }
        case glLoadMatrixf_INDEX:
        case glLoadTransposeMatrixf_INDEX:
        case glMultMatrixf_INDEX:
        case glMultTransposeMatrixf_INDEX:

        {
            glLoadMatrixf_PACKED *n = (glLoadMatrixf_PACKED *)call;
            ring_write(ring, n->args.m, 16 * sizeof(GLfloat));
            break;
        }
        case glLightfv_INDEX:
        {
            glLightfv_PACKED *n = (glLightfv_PACKED *)call;
            int count = 1;
            switch (n->args.pname) {
                case GL_AMBIENT:
                case GL_DIFFUSE:
                case GL_SPECULAR:
                case GL_POSITION:
                    count = 4;
                    break;
                case GL_SPOT_DIRECTION:
                    count = 3;
                    break;
            }
            ring_write(ring, n->args.params, count * sizeof(GLfloat));
            break;
        }
        case glMaterialfv_INDEX:
        {
            glMaterialfv_PACKED *n = (glMaterialfv_PACKED *)call;
            int count = 4;
            switch (n->args.pname) {
                case GL_SHININESS:
                    count = 1;
                    break;
                case GL_COLOR_INDEXES:
                    count = 3;
                    break;
            }
            ring_write(ring, n->args.params, count * sizeof(GLfloat));
            break;
        }
        case glBitmap_INDEX:
        {
            glBitmap_PACKED *n = (glBitmap_PACKED *)call;
            size_t size = ((n->args.width + 7) / 8) * n->args.height;
            ring_write(ring, n->args.bitmap, size);
            break;
        }
        case glDrawPixels_INDEX:
        {
            glDrawPixels_PACKED *n = (glDrawPixels_PACKED *)call;
            size_t size = n->args.width * n->args.height * gl_pixel_sizeof(n->args.format, n->args.type);
            ring_write(ring, n->args.pixels, size);
            break;
        }
        case glFogiv_INDEX:
        {
            glFogiv_PACKED *n = (glFogiv_PACKED *)call;
            ring_write(ring, n->args.params, gl_fogv_length(n->args.pname) * sizeof(GLint));
            break;
        }
        case glFogfv_INDEX:
        {
            glFogfv_PACKED *n = (glFogfv_PACKED *)call;
            ring_write(ring, n->args.params, gl_fogv_length(n->args.pname) * sizeof(GLint));
            break;
        }
#if 0
        // this is disabled to remove the X dependency for now
        // it looks like glXChooseVisual returns don't matter anyway
        case glXChooseVisual_INDEX:
        {
            glXChooseVisual_PACKED *n = (glXChooseVisual_PACKED *)call;
            int *attribList = n->args.attribList;
            int size = 0;
            while (attribList[size++]) {}
            ring_write(ring, attribList, size);
            break;
        }
#endif
    }
}

void remote_local_post(ring_t *ring, packed_call_t *call, void *ret_v, size_t ret_size) {
    switch (call->index) {
        case glGenTextures_INDEX:
            ring_read_into(ring, ((glGenTextures_PACKED *)call)->args.textures);
            break;
        case glGetString_INDEX:
        {
            char *str = ring_read(ring, NULL);
            // TODO: somehow free this, or use a klist to preserve
            char *ret = malloc(strlen(str));
            strcpy(ret, str);
            *(char **)ret_v = ret;
            break;
        }
        case glXSwapBuffers_INDEX:
            // prevent too many frames from clogging up the ring buffer
            ring_read(ring, NULL);
            break;
#if 0
        // see above
        case glXChooseVisual_INDEX:
        {
            glXChooseVisual_PACKED *n = (glXChooseVisual_PACKED *)call;
            XVisualInfo **ret_vis = (XVisualInfo **)ret_v;
            if (*ret_vis) {
                XVisualInfo *visual = ret->arg[1].data.block.data;
                int count;
                XVisualInfo tmp;
                XMatchVisualInfo(n->args.dpy, visual->screen, visual->depth, visual->class, &tmp);
                memcpy(visual, &tmp, sizeof(XVisualInfo));
                *ret_vis = visual;
            }
            break;
        }
#endif
    }
}

void remote_target_pre(ring_t *ring, packed_call_t *call, size_t size, void *ret) {
    static Display *target_display = NULL;
    if (! target_display) target_display = XOpenDisplay(NULL);
    switch (call->index) {
        // can't pass a Display * through the proxy
        case glXCreateContext_INDEX:
        {
            glXCreateContext_PACKED *n = (glXCreateContext_PACKED *)call;
            n->args.dpy = target_display;
            n->args.vis = ring_read(ring, NULL);
            if (n->args.shareList) {
                n->args.shareList = ring_read(ring, NULL);
            }
            break;
        }
        case glXMakeCurrent_INDEX:
            ((glXCreateContext_PACKED *)call)->args.dpy = target_display;
            break;
        case glXDestroyContext_INDEX:
            ((glXDestroyContext_PACKED *)call)->args.dpy = target_display;
            break;
        case glXSwapBuffers_INDEX:
            ring_write(ring, NULL, 0);
            ((glXSwapBuffers_PACKED *)call)->args.dpy = target_display;
            break;
        case REMOTE_BLOCK_DRAW:
        {
            if (state.list.active) {
                void *tmp = malloc(size);
                memcpy(tmp, call, size);
                block_t *block = remote_read_block(ring, tmp);
                dl_append_block(state.list.active, block);
                block->solid = tmp;
            } else {
                block_t *block = remote_read_block(ring, (void *)call);
                bl_draw(block);
            }
            return;
        }
        case REMOTE_GL_GET:
        {
            uint32_t *buf = (uint32_t *)call;
            gl_get(buf[1], buf[2], ret);
            return;
        }
        case glDeleteTextures_INDEX:
            ((glDeleteTextures_PACKED *)call)->args.textures = ring_read(ring, NULL);
            break;
        case glTexImage2D_INDEX:
        {
            glTexImage2D_PACKED *n = (glTexImage2D_PACKED *)call;
            if (n->args.pixels)
                n->args.pixels = ring_read(ring, NULL);
            break;
        }
        case glTexSubImage2D_INDEX:
        {
            glTexSubImage2D_PACKED *n = (glTexSubImage2D_PACKED *)call;
            if (n->args.pixels)
                n->args.pixels = ring_read(ring, NULL);
            break;
        }
        case glLoadMatrixf_INDEX:
        case glLoadTransposeMatrixf_INDEX:
        case glMultMatrixf_INDEX:
        case glMultTransposeMatrixf_INDEX:
            ((glLoadMatrixf_PACKED *)call)->args.m = ring_read(ring, NULL);
            break;
        case glLightfv_INDEX:
            ((glLightfv_PACKED *)call)->args.params = ring_read(ring, NULL);
            break;
        case glMaterialfv_INDEX:
            ((glMaterialfv_PACKED *)call)->args.params = ring_read(ring, NULL);
            break;
        case glBitmap_INDEX:
            ((glBitmap_PACKED *)call)->args.bitmap = ring_read(ring, NULL);
            break;
        case glDrawPixels_INDEX:
            ((glDrawPixels_PACKED *)call)->args.pixels = ring_read(ring, NULL);
            break;
        case glGenTextures_INDEX:
        {
            glGenTextures_PACKED *n = (glGenTextures_PACKED *)call;
            size_t size = n->args.n * sizeof(GLuint);
            GLuint *textures = ring_dma(ring, size);
            glGenTextures_PACKED tmp = {glGenTextures_INDEX, {n->args.n, textures}};
            glIndexedCall((packed_call_t *)&tmp, NULL);
            ring_dma_done(ring);
            return;
        }
        case glFogiv_INDEX:
            ((glFogiv_PACKED *)call)->args.params = ring_read(ring, NULL);
            break;
        case glFogfv_INDEX:
            ((glFogfv_PACKED *)call)->args.params = ring_read(ring, NULL);
            break;
#if 0
        // see above
        case glXChooseVisual_INDEX:
        {
            PACKED_glXChooseVisual *n = (PACKED_glXChooseVisual *)call;
            n->args.attribList = ring_read(ring, NULL);
            int *attribList = n->args.attribList;
            XVisualInfo *info = NULL;
            glIndexedCall(call, (void *)&info);
            if (info) {
                ring_write(ring, info, sizeof(XVisualInfo));
            }
            return;
        }
#endif
    }
    glIndexedCall(call, ret);
}

void remote_target_post(ring_t *ring, packed_call_t *call, void *ret) {
    switch (call->index) {
        case glGetString_INDEX:
        {
            char *str = *(char **)ret;
            ring_write(ring, str, strlen(str) + 1);
            return;
        }
    }
}
