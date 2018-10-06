#include "glx.h"

#define MAP(func_name, func) \
    if (strcmp(name, func_name) == 0) return (void *)func;

#define MAP_EGL(func_name, egl_func) \
    MAP(#func_name, eglGetProcAddress(#egl_func))

#define EX(func_name) MAP(#func_name, func_name)

#define ARB(func_name) MAP(#func_name "ARB", func_name)

#define STUB(func_name)                       \
    if (strcmp(name, #func_name) == 0) {      \
        printf("glX stub: %s\n", #func_name); \
        return (void *)glXStub;               \
    }

void glXStub(void *x, ...) {
    return;
}

void *glXGetProcAddressARB(const char *name) {
    // generated gles wrappers
#ifdef USE_ES2
    #include "gles2funcs.inc"
#else
    #include "glesfuncs.inc"
#endif

    // glX calls
    EX(glXChooseVisual);
    EX(glXCopyContext);
    EX(glXCreateContext);
    EX(glXCreateContextAttribsARB);
    EX(glXCreateGLXPixmap);
    EX(glXDestroyContext);
    EX(glXDestroyGLXPixmap);
    EX(glXGetConfig);
    EX(glXGetCurrentDisplay);
    EX(glXGetCurrentDrawable);
    EX(glXIsDirect);
    EX(glXMakeCurrent);
    EX(glXQueryExtensionsString);
    EX(glXQueryServerString);
    EX(glXSwapBuffers);
    EX(glXSwapIntervalEXT);
    EX(glXSwapIntervalMESA);
    EX(glXSwapIntervalSGI);
    EX(glXUseXFont);
    EX(glXWaitGL);
    EX(glXWaitX);

    // GL_ARB_vertex_buffer_object
/*
    ARB(glBindBuffer);
    ARB(glBufferData);
    ARB(glBufferSubData);
    ARB(glDeleteBuffers);
    ARB(glGenBuffers);
    ARB(glIsBuffer);
    MAP_EGL(glGetBufferParameteriARB, glGetBufferParameteriOES);
    MAP_EGL(glGetBufferPointerARB, glGetBufferPointerOES);
    MAP_EGL(glGetBufferPointervARB, glGetBufferPointervOES);
    MAP_EGL(glMapBufferARB, glMapBufferOES);
    MAP_EGL(glUnmapBufferARB, glMapBufferOES);
    STUB(glGetBufferParameterivARB);
    STUB(glGetBufferSubDataARB);
*/

    // OES wrapper
    EX(glClearDepthfOES);
    EX(glClipPlanefOES);
    EX(glDepthRangefOES);
    EX(glFrustumfOES);
    EX(glGetClipPlanefOES);
    EX(glOrthofOES);

    // passthrough
    // batch thunking!
    #define THUNK(suffix, type)       \
    EX(glColor3##suffix##v);          \
    EX(glColor3##suffix);             \
    EX(glColor4##suffix##v);          \
    EX(glColor4##suffix);             \
    EX(glSecondaryColor3##suffix##v); \
    EX(glSecondaryColor3##suffix);    \
    MAP("glSecondaryColor3" #suffix "vEXT", glSecondaryColor3##suffix##v); \
    MAP("glSecondaryColor3" #suffix "EXT",  glSecondaryColor3##suffix);    \
    EX(glIndex##suffix##v);           \
    EX(glIndex##suffix);              \
    EX(glNormal3##suffix##v);         \
    EX(glNormal3##suffix);            \
    EX(glRasterPos2##suffix##v);      \
    EX(glRasterPos2##suffix);         \
    EX(glRasterPos3##suffix##v);      \
    EX(glRasterPos3##suffix);         \
    EX(glRasterPos4##suffix##v);      \
    EX(glRasterPos4##suffix);         \
    EX(glVertex2##suffix##v);         \
    EX(glVertex2##suffix);            \
    EX(glVertex3##suffix##v);         \
    EX(glVertex3##suffix);            \
    EX(glVertex4##suffix##v);         \
    EX(glVertex4##suffix);            \
    EX(glTexCoord1##suffix##v);       \
    EX(glTexCoord1##suffix);          \
    EX(glTexCoord2##suffix##v);       \
    EX(glTexCoord2##suffix);          \
    EX(glTexCoord3##suffix##v);       \
    EX(glTexCoord3##suffix);          \
    EX(glTexCoord4##suffix##v);       \
    EX(glTexCoord4##suffix);

    THUNK(b, GLbyte);
    THUNK(d, GLdouble);
    THUNK(i, GLint);
    THUNK(s, GLshort);
    THUNK(ub, GLubyte);
    THUNK(ui, GLuint);
    THUNK(us, GLushort);
    THUNK(f, GLfloat);
    #undef THUNK

#ifdef USE_ES2
    EX(glCompileShaderARB);
    EX(glCreateShaderObjectARB);
    EX(glGetObjectParameterivARB);
    EX(glShaderSourceARB);
#endif

    // functions we actually define
    EX(glActiveTextureARB);
    EX(glArrayElement);
    EX(glBegin);
    EX(glBitmap);
    EX(glBlendEquationSeparatei);
    EX(glBlendFuncSeparate);
    EX(glBlendFuncSeparatei);
    EX(glCallList);
    EX(glCallLists);
    EX(glClearDepth);
    EX(glClientActiveTextureARB);
    EX(glClipPlane);
    EX(glCopyPixels);
    EX(glDeleteLists);
    EX(glDepthRange);
    EX(glDisable);
    EX(glDrawBuffer);
    EX(glDrawPixels);
    EX(glEdgeFlag);
    EX(glEnable);
    EX(glEnd);
    EX(glEndList);
    EX(glEvalCoord1d);
    EX(glEvalCoord1f);
    EX(glEvalCoord2d);
    EX(glEvalCoord2f);
    EX(glEvalMesh1);
    EX(glEvalMesh2);
    EX(glEvalPoint1);
    EX(glEvalPoint2);
    EX(glFogCoordd);
    EX(glFogCoorddv);
    EX(glFogCoordf);
    EX(glFogCoordfv);
    EX(glFogi);
    EX(glFogiv);
    EX(glFrustum);
    EX(glGenLists);
    EX(glGetDoublev);
    EX(glGetIntegerv);
    EX(glGetMapdv);
    EX(glGetMapfv);
    EX(glGetMapiv);
    EX(glGetTexImage);
    EX(glGetTexLevelParameterfv);
    EX(glGetTexLevelParameteriv);
    EX(glInitNames);
    EX(glInterleavedArrays);
    EX(glIsList);
#ifndef USE_ES2
    EX(glLighti);
    EX(glLightiv);
    EX(glLightModeli);
    EX(glLightModeliv);
#endif
    EX(glLineStipple);
    EX(glListBase);
    EX(glLoadMatrixd);
    EX(glLoadName);
    EX(glLockArraysEXT);
    EX(glMap1d);
    EX(glMap1f);
    EX(glMap2d);
    EX(glMap2f);
    EX(glMapGrid1d);
    EX(glMapGrid1f);
    EX(glMapGrid2d);
    EX(glMapGrid2f);
    EX(glMateriali);
    EX(glMultiTexCoord2f);
    EX(glMultMatrixd);
    EX(glNewList);
    EX(glOrtho);
    EX(glPixelTransferf);
    EX(glPixelTransferi);
    EX(glPixelZoom);
    EX(glPolygonMode);
    EX(glPolygonStipple);
    EX(glPopAttrib);
    EX(glPopClientAttrib);
    EX(glPopName);
    EX(glPushAttrib);
    EX(glPushClientAttrib);
    EX(glPushName);
    EX(glRasterPos2i);
    EX(glReadBuffer);
    EX(glRectd);
    EX(glRectf);
    EX(glRecti);
    EX(glRects);
    EX(glRenderMode);
    EX(glRotated);
    EX(glScaled);
    EX(glTexGend);
    EX(glTexGendv);
    EX(glTexGenf);
    EX(glTexGenfv);
    EX(glTexGeni);
    EX(glTexGeniv);
    EX(glTexImage1D);
    EX(glTexImage3D);
    EX(glTranslated);
    EX(glUnlockArraysEXT);

    // stubs for unimplemented functions
    STUB(glAccum);
    STUB(glAreTexturesResident);
    STUB(glClearAccum);
    STUB(glColorMaterial);
    STUB(glCopyTexImage1D);
    STUB(glCopyTexSubImage1D);
    STUB(glEdgeFlagPointer);
    STUB(glFeedbackBuffer);
    STUB(glGetClipPlane);
    STUB(glGetLightiv);
    STUB(glGetMaterialiv);
    STUB(glGetPixelMapfv);
    STUB(glGetPixelMapuiv);
    STUB(glGetPixelMapusv);
    STUB(glGetPolygonStipple);
    STUB(glGetStringi);
    STUB(glGetTexGendv);
    STUB(glGetTexGenfv);
    STUB(glGetTexGeniv);
    STUB(glMaterialiv);
    STUB(glPassThrough);
    STUB(glPixelMapfv);
    STUB(glPixelMapuiv);
    STUB(glPixelMapusv);
    STUB(glPixelStoref);
    STUB(glPrioritizeTextures);
    STUB(glSelectBuffer);
    STUB(glTexSubImage1D);

    printf("glXGetProcAddress: %s not found.\n", name);
    return NULL;
}

void *glXGetProcAddress(const char *name) {
    return glXGetProcAddressARB(name);
}
