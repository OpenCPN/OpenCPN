#include "line.h"

GLint stippleFactor = 1;
GLushort stipplePattern = 0xFFFF;
GLubyte *stippleData = NULL;
GLuint stippleTexture = 0;

void glLineStipple(GLuint factor, GLushort pattern) {
    /* glTexImage2D is not allowed in a display list, but glLineStipple is,
       this causes it to clobber the current texture since binding stippleTexture is
       only added to the list.

       for now, simply abort to avoid this case */
    if (state.list.active)
        return;

    stippleFactor = factor;
    stipplePattern = pattern;
    if (stippleData != NULL) {
        free(stippleData);
    }
    stippleData = (GLubyte *)malloc(sizeof(GLubyte) * 16);
    for (int i = 0; i < 16; i++) {
        stippleData[i] = (stipplePattern >> i) & 1 ? 255 : 0;
    }

    glPushAttrib(GL_TEXTURE_BIT);
    if (! stippleTexture)
        glGenTextures(1, &stippleTexture);

    glBindTexture(GL_TEXTURE_2D, stippleTexture);
    glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_ALPHA,
        16, 1, 0, GL_ALPHA, GL_UNSIGNED_BYTE, stippleData);
    glPopAttrib();
}

void bind_stipple_tex() {
    glBindTexture(GL_TEXTURE_2D, stippleTexture);
}

GLfloat *gen_stipple_tex_coords(GLfloat *vert, int length) {
    // generate our texture coords
    GLfloat *tex = (GLfloat *)malloc(length * 2 * sizeof(GLfloat));
    GLfloat *texPos = tex;
    GLfloat *vertPos = vert;

    GLfloat x1, x2, y1, y2;
    GLfloat len;
    for (int i = 0; i < length / 2; i++) {
        x1 = *vertPos++;
        y1 = *vertPos++;
        vertPos++; // z
        x2 = *vertPos++;
        y2 = *vertPos++;
        vertPos++;

        len = sqrt(pow(x2-x1, 2) + pow(y2-y1, 2)) / stippleFactor * 16;

        *texPos++ = 0;
        *texPos++ = 0;
        *texPos++ = len;
        *texPos++ = 0;
    }
    return tex;
}
