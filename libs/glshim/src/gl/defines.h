#include <GL/gl.h>

// newly-defined GL functions
GLboolean glIsList(GLuint list);
GLuint glGenLists(GLsizei range);
void glActiveTextureARB(GLenum texture);
void glArrayElement(GLint i);
void glBegin(GLenum mode);
void glCallList(GLuint list);
void glCallLists(GLsizei n, GLenum type, const GLvoid *lists);
void glClearDepth(GLdouble depth);
void glDeleteList(GLuint list);
void glDeleteLists(GLuint list, GLsizei range);
void glDrawArrays(GLenum mode, GLint first, GLsizei count);
void glEnd();
void glEndList();
void glFrustum(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near, GLdouble far);
void glGetDoublev(GLenum pname, GLdouble *params);
void glIndexf(GLfloat i);
void glInterleavedArrays(GLenum format, GLsizei stride, const GLvoid *pointer);
void glListBase(GLuint base);
void glLockArraysEXT(GLint first, GLsizei count);
void glNewList(GLuint list, GLenum mode);
void glOrtho(GLdouble left, GLdouble right, GLdouble bottom, GLdouble top, GLdouble near, GLdouble far);
void glSecondaryColor3f(GLfloat r, GLfloat g, GLfloat b);
void glTexCoord2f(GLfloat s, GLfloat t);
void glUnlockArraysEXT();
void glVertex2f(GLfloat x, GLfloat y);
void glVertex2i(GLint x, GLint y);
void glVertex3f(GLfloat x, GLfloat y, GLfloat z);

// custom functions
void glPushCall(void *call);
