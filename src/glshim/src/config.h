#define SYS_proxy 9999
#define MAX_EVAL_ORDER 30

// don't auto-wrap these functions
#define skip_glColor4ub

// gl.c
#define skip_glColor4f
#define skip_glDisable
#define skip_glEnable
#define skip_glGetIntegerv
#define skip_glGetString
#define skip_glIsEnabled
#define skip_glMaterialfv
#define skip_glNormal3f

// light.c
#define skip_glLightModelf

// raster.c
#define skip_glViewport

// texture.c
#define skip_glBindTexture
#define skip_glDeleteTextures
#define skip_glPixelStorei
#define skip_glTexImage2D
#define skip_glTexParameteri
#define skip_glTexSubImage2D

// glDrawArrays
#define skip_glDrawArrays
#define skip_glDrawElements
#define skip_glVertexPointer
#define skip_glColorPointer
#define skip_glNormalPointer
#define skip_glTexCoordPointer
#define skip_glDisableClientState
#define skip_glEnableClientState


// don't compile these into display lists
#define direct_glColorPointer
#define direct_glDeleteLists
#define direct_glDisableClientState
#define direct_glEdgeFlagPointer
#define direct_glEnableClientState
#define direct_glFeedbackBuffer
#define direct_glFinish
#define direct_glFlush
#define direct_glGenLists
#define direct_glIndexPointer
#define direct_glInterleavedArrays
#define direct_glIsEnabled
#define direct_glIsList
#define direct_glNormalPointer
#define direct_glPopClientAttrib
#define direct_glPixelStore
#define direct_glPushClientAttrib
#define direct_glReadPixels
#define direct_glRenderMode
#define direct_glSelectBuffer
#define direct_glTexCoordPointer
#define direct_glVertexPointer
