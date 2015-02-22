#include <stdio.h>
#include "../proxy/gl.h"

int main() {
    glXSwapBuffers(0, 0);
    glColor4f(1, 2, 3, 4);
    glBegin(GL_QUADS);
    glVertex2f(1, 2);
    glEnd();
}
