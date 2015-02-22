#include "raster.h"

rasterpos_t rPos = {0, 0, 0};
viewport_t viewport = {0, 0, 0, 0};
GLubyte *raster = NULL;

/* raster engine:
    we render pixels to memory somewhere
    until someone else wants to use the framebuffer
    then we throw 'em quickly into a texture, render to the whole screen
    then let the other function do their thing
*/

void glRasterPos3f(GLfloat x, GLfloat y, GLfloat z) {
    rPos.x = x;
    rPos.y = y;
    rPos.z = z;
}

void glViewport(GLint x, GLint y, GLsizei width, GLsizei height) {
    PUSH_IF_COMPILING(glViewport);
    LOAD_GLES(glViewport);
    if (raster) {
        render_raster();
    }
    gles_glViewport(x, y, width, height);
    viewport.x = x;
    viewport.y = y;
    viewport.width = width;
    viewport.height = height;
}

void init_raster() {
    if (!viewport.width || !viewport.height) {
        glGetIntegerv(GL_VIEWPORT, (GLint *)&viewport);
    }
    if (!raster)
        raster = (GLubyte *)malloc(4 * viewport.width * viewport.height * sizeof(GLubyte));
}

void glBitmap(GLsizei width, GLsizei height, GLfloat xorig, GLfloat yorig,
              GLfloat xmove, GLfloat ymove, const GLubyte *bitmap) {
    // TODO: shouldn't be drawn if the raster pos is outside the viewport?
    // TODO: negative width/height mirrors bitmap?
    if (!width && !height) {
        rPos.x += xmove;
        rPos.y -= ymove;
        return;
    }
    init_raster();

    const GLubyte *from;
    GLubyte *to;
    int x, y;

    // copy to pixel data
    // TODO: strip blank lines and mirror vertically?
    for (y = 0; y < height; y++) {
        to = raster + 4 * (GLuint)(rPos.x + ((rPos.y - y) * viewport.width));
        from = bitmap + (y * 2);
        for (x = 0; x < (width + 7 / 8); x++) {
            if (rPos.x + x > viewport.width || rPos.y + y > viewport.height)
                continue;
            // TODO: wasteful, unroll this?
            GLubyte b = from[(x / 8)];
            int p = (b & (1 << (7 - (x % 8)))) ? 1 : 0;
            // r, g, b, a
            p = (p ? 255 : 0);
            *to++ = p;
            *to++ = p;
            *to++ = p;
            *to++ = p;
        }
    }

    rPos.x += xmove;
    rPos.y += ymove;
}

void glDrawPixels(GLsizei width, GLsizei height, GLenum format,
                  GLenum type, const GLvoid *data) {
    GLubyte *pixels, *from, *to;
    GLvoid *dst = NULL;

    init_raster();
    if (! pixel_convert(data, &dst, width, height,
                        format, type, GL_RGBA, GL_UNSIGNED_BYTE)) {
        return;
    }
    pixels = (GLubyte *)dst;

    for (int y = 0; y < height; y++) {
        if(rPos.y - y < 0 || rPos.y - y >= viewport.height)
            continue;

        to = raster + 4 * (GLuint)(rPos.x + ((rPos.y - y) * viewport.width));
        from = pixels + 4 * (y * width);
        for (int x = 0; x < width; x++) {
            if(rPos.x + x < 0 || rPos.x + x >= viewport.width)
                continue;

            *to++ = *from++;
            *to++ = *from++;
            *to++ = *from++;
            *to++ = *from++;
        }
    }
    if (pixels != data)
        free(pixels);
}

void render_raster() {
    if (!viewport.width || !viewport.height || !raster)
        return;

// FIXME
#ifndef USE_ES2
    glPushAttrib(GL_TEXTURE_BIT | GL_ENABLE_BIT);
    glMatrixMode(GL_PROJECTION);
    glPushMatrix();
    glLoadIdentity();
    glMatrixMode(GL_MODELVIEW);
    glPushMatrix();
    glLoadIdentity();

    GLfloat vert[] = {
        -1, -1, 0,
        1, -1, 0,
        1, 1, 0,
        -1, 1, 0,
    };

    float sw = viewport.width / (GLfloat)npot(viewport.width);
    float sh = viewport.height / (GLfloat)npot(viewport.height);

    GLfloat tex[] = {
        0, sh,
        sw, sh,
        sw, 0,
        0, 0,
    };

    glPushClientAttrib(GL_CLIENT_VERTEX_ARRAY_BIT | GL_CLIENT_PIXEL_STORE_BIT);
    glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
    glVertexPointer(3, GL_FLOAT, 0, vert);
    glTexCoordPointer(2, GL_FLOAT, 0, tex);

    glEnable(GL_BLEND);
    glEnable(GL_TEXTURE_2D);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    GLuint texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glPixelStorei(GL_UNPACK_SKIP_PIXELS, 0);
    glPixelStorei(GL_UNPACK_SKIP_ROWS, 0);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, npot(viewport.width), npot(viewport.height),
                 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
    glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, viewport.width, viewport.height,
                    GL_RGBA, GL_UNSIGNED_BYTE, raster);

    LOAD_GLES(glDrawArrays);
    gles_glDrawArrays(GL_TRIANGLE_FAN, 0, 4);
    glDeleteTextures(1, &texture);

    glDisable(GL_BLEND);
    glDisable(GL_TEXTURE_2D);
    glDisableClientState(GL_VERTEX_ARRAY);
    glDisableClientState(GL_TEXTURE_COORD_ARRAY);
    glPopClientAttrib();

    glPopMatrix();
    glMatrixMode(GL_PROJECTION);
    glPopMatrix();
    glPopAttrib();
#endif
    free(raster);
    raster = NULL;
}
