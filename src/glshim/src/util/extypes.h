#ifndef GL_EXTYPES_H
#define GL_EXTYPES_H

#include <stdint.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

// gles

typedef void *GLDEBUGPROC;
typedef int32_t GLclampx;
typedef int32_t GLfixed;

// glx + x11

typedef void *DMbuffer;
typedef void *GLXContextID;
typedef int GLXDrawable;
typedef void *GLXFBConfigSGIX;
typedef void *GLXHyperpipeConfigSGIX;
typedef void *GLXHyperpipeNetworkSGIX;
typedef void *GLXPbuffer;
typedef void *GLXPbufferSGIX;
typedef void *GLXPixmap;
typedef void *GLXVideoCaptureDeviceNV;
typedef void *GLXVideoDeviceNV;
typedef void *GLXVideoSourceSGIX;
typedef void *GLXWindow;
typedef void *VLNode;
typedef void *VLPath;
typedef void *VLServer;
typedef void *__GLXextFuncPtr;
typedef void DMparams;

struct __GLXContextRec {
    Display *display;
    unsigned char direct;
    int currentWritable;
    int currentReadable;
    XID xid;
};
typedef struct __GLXContextRec *GLXContext;

struct __GLXFBConfigRec {
    int visualType;
    int transparentType;
                                /*    colors are floats scaled to ints */
    int transparentRed, transparentGreen, transparentBlue, transparentAlpha;
    int transparentIndex;

    int visualCaveat;

    int associatedVisualId;
    int screen;

    int drawableType;
    int renderType;

    int maxPbufferWidth, maxPbufferHeight, maxPbufferPixels;
    int optimalPbufferWidth, optimalPbufferHeight;  /* for SGIX_pbuffer */

    int visualSelectGroup;  /* visuals grouped by select priority */

    unsigned int id;

    unsigned char rgbMode;
    unsigned char colorIndexMode;
    unsigned char doubleBufferMode;
    unsigned char stereoMode;
    unsigned char haveAccumBuffer;
    unsigned char haveDepthBuffer;
    unsigned char haveStencilBuffer;

    /* The number of bits present in various buffers */
    int accumRedBits, accumGreenBits, accumBlueBits, accumAlphaBits;
    int depthBits;
    int stencilBits;
    int indexBits;
    int redBits, greenBits, blueBits, alphaBits;
    unsigned int redMask, greenMask, blueMask, alphaMask;

    unsigned int multiSampleSize; /* Number of samples per pixel (0 if no ms) */

    unsigned int nMultiSampleBuffers; /* Number of availble ms buffers */
    int maxAuxBuffers;

    /* frame buffer level */
    int level;

    /* color ranges (for SGI_color_range) */
    unsigned char extendedRange;
    double minRed, maxRed;
    double minGreen, maxGreen;
    double minBlue, maxBlue;
    double minAlpha, maxAlpha;
};
typedef struct __GLXFBConfigRec *GLXFBConfig;

// egl
#if !defined(__unix__) && defined(__APPLE__) && defined(__MACH__)
#define __unix__
#include <EGL/egl.h>
#include <EGL/eglext.h>
#undef __unix__
#else
#include <EGL/egl.h>
#include <EGL/eglext.h>
#endif
#endif
