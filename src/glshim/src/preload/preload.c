#define _GNU_SOURCE

#include <SDL/SDL.h>
#include <dlfcn.h>
#include <X11/Xlib.h>
#include <stdio.h>

/*
SDL_GrabMode SDL_WM_GrabInput(SDL_GrabMode mode) {
    printf("SDL_WM_GrabInput\n");
    return mode;
}
int SDL_ShowCursor(int toggle) {
    printf("SDL_ShowCursor(%i)\n", toggle);
    return toggle;
}
void SDL_WarpMouse(Uint16 x, Uint16 y) {
    printf("SDL_WM_WarpMouse\n");
}
int SDL_PollEvent(SDL_Event *event) {
    return 0;
}
*/

int SDL_SetGamma(float r, float g, float b) {
    printf("-!- Skipping SDL_SetGamma(%.2f, %.2f, %.2f)\n", r, g, b);
    return 0;
}

static Display *g_display;
static int displayRefs = 0;
typedef Display *(*XOPENDISPLAYPTR)(const char *);
Display *XOpenDisplay(const char *display_name) {
    static XOPENDISPLAYPTR real_XOpenDisplay;
    if (!real_XOpenDisplay) {
        real_XOpenDisplay = (XOPENDISPLAYPTR)dlsym(RTLD_NEXT, "XOpenDisplay");
    }
    if (!g_display) {
        g_display = real_XOpenDisplay(display_name);
    }
    displayRefs++;
    return g_display;
}

typedef int (*XCLOSEDISPLAYPTR)(Display *);
int XCloseDisplay(Display *display) {
    static XCLOSEDISPLAYPTR real_XCloseDisplay;
    if (!real_XCloseDisplay) {
        real_XCloseDisplay = (XCLOSEDISPLAYPTR)dlsym(RTLD_NEXT, "XCloseDisplay");
    }
    if (g_display == display && displayRefs > 0) {
        if (displayRefs-- == 0) {
            g_display = NULL;
            return real_XCloseDisplay(display);
        }
    }
    return 0;
}

/* Pandora crashes on mouse events caused by libxi. Workaround by overriding this... */
int XIQueryVersion(void *display, int *major, int *minor) {
    printf("-!- Skipping Xinput(%d, %d) query for display: %p\n", (major?*major:0), (minor?*minor:0), display);
    return 1; // BadRequest
}

/*
typedef int (*XSYNCPTR)(Display *, Bool);
int XSync(Display *display, Bool discard) {
    // discard = 1;

    static XSYNCPTR real_XSync;
    if (!real_XSync) {
        real_XSync = (XSYNCPTR)dlsym(RTLD_NEXT, "XSync");
    }
    return real_XSync(display, discard);
}
*/
