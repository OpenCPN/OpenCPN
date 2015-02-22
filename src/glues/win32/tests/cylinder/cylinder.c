/*                                                              */
/* This test is based on accumaa.c - by Tom McReynolds, SGI and */
/* initialization part on PowerVR SDK examples                  */
/*                                                              */
/* // Mike Gorchak, 2009. GLU ES test                           */
/*                                                              */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <windows.h>
#include <TCHAR.h>

#include <GLES\gl.h>
#include <GLES\egl.h>

#include "glues.h"

#define WINDOW_WIDTH   640
#define WINDOW_HEIGHT  480

#define	WINDOW_CLASS _T("GLUES")

GLUquadricObj* cone_fill_smooth;
GLUquadricObj* cone_fill_flat;
GLUquadricObj* cone_fill_texture;
GLUquadricObj* cone_point_smooth;
GLUquadricObj* cone_point_flat;
GLUquadricObj* cone_point_texture;
GLUquadricObj* cone_line_smooth;
GLUquadricObj* cone_line_flat;
GLUquadricObj* cone_line_texture;
GLfloat rotate=0;

/* Create a single component texture map */
GLubyte* make_texture(int maxs, int maxt)
{
   int s, t;
   static GLubyte *texture;

   texture=(GLubyte*)malloc(maxs*maxt*sizeof(GLubyte));
   for (t=0; t<maxt; t++)
   {
      for (s=0; s<maxs; s++)
      {
         texture[s+maxs*t]=(((s>>4)&0x1)^((t>>4)&0x1))*255;
      }
   }

   return texture;
}


void init_scene()
{
   static GLfloat lightpos[4]={50.0f, 50.0f, -320.f, 1.0f};
   GLubyte* tex;

   /* Clear error */
   glGetError();

   /* draw a perspective scene */
   glMatrixMode(GL_PROJECTION);
   glFrustumf(-100.f, 100.f, -100.f, 100.f, 320.f, 640.f);
   glMatrixMode(GL_MODELVIEW);

   /* turn on features */
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);

   /* place light 0 in the right place */
   glLightfv(GL_LIGHT0, GL_POSITION, lightpos);

   /* enable filtering */
   glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

   tex=make_texture(256, 256);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 256, 256, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, tex);
   free(tex);

   cone_fill_smooth=gluNewQuadric();
   gluQuadricDrawStyle(cone_fill_smooth, GLU_FILL);
   gluQuadricNormals(cone_fill_smooth, GLU_SMOOTH);

   cone_fill_flat=gluNewQuadric();
   gluQuadricDrawStyle(cone_fill_flat, GLU_FILL);
   gluQuadricNormals(cone_fill_flat, GLU_FLAT);

   cone_fill_texture=gluNewQuadric();
   gluQuadricDrawStyle(cone_fill_texture, GLU_FILL);
   gluQuadricNormals(cone_fill_texture, GLU_SMOOTH);
   gluQuadricTexture(cone_fill_texture, GLU_TRUE);

   cone_point_smooth=gluNewQuadric();
   gluQuadricDrawStyle(cone_point_smooth, GLU_POINT);
   gluQuadricNormals(cone_point_smooth, GLU_SMOOTH);

   cone_point_flat=gluNewQuadric();
   gluQuadricDrawStyle(cone_point_flat, GLU_POINT);
   gluQuadricNormals(cone_point_flat, GLU_FLAT);

   cone_point_texture=gluNewQuadric();
   gluQuadricDrawStyle(cone_point_texture, GLU_POINT);
   gluQuadricNormals(cone_point_texture, GLU_SMOOTH);
   gluQuadricTexture(cone_point_texture, GLU_TRUE);

   cone_line_smooth=gluNewQuadric();
   gluQuadricDrawStyle(cone_line_smooth, GLU_LINE);
   gluQuadricNormals(cone_line_smooth, GLU_SMOOTH);

   cone_line_flat=gluNewQuadric();
   gluQuadricDrawStyle(cone_line_flat, GLU_LINE);
   gluQuadricNormals(cone_line_flat, GLU_FLAT);

   cone_line_texture=gluNewQuadric();
   gluQuadricDrawStyle(cone_line_texture, GLU_LINE);
   gluQuadricNormals(cone_line_texture, GLU_SMOOTH);
   gluQuadricTexture(cone_line_texture, GLU_TRUE);

   if (glGetError())
   {
      printf("Oops! I screwed up my OpenGL ES calls somewhere\n");
   }
}

void render_scene()
{
   /* material properties for objects in scene */
   static GLfloat wall_mat[4]={1.0f, 1.0f, 1.0f, 1.0f};
   static GLfloat cone_mat[4]={0.0f, 0.5f, 1.0f, 1.0f};
   GLfloat texcoords[4][2];
   GLfloat vertices[4][3];

   glShadeModel(GL_SMOOTH);

   glVertexPointer(3, GL_FLOAT, 0, vertices);
   glTexCoordPointer(2, GL_FLOAT, 0, texcoords);

   /* Enable vertices and texcoords arrays */
   glEnableClientState(GL_VERTEX_ARRAY);
   glEnableClientState(GL_TEXTURE_COORD_ARRAY);

   glGetError();
   glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

   /* Note: wall verticies are ordered so they are all front facing this lets
      me do back face culling to speed things up.  */
   glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, wall_mat);

   /* floor */
   glEnable(GL_TEXTURE_2D);

   glNormal3f(0.f, 1.f, 0.f);

   /* Fill texture coordinates and vertices arrays */
   texcoords[0][0]=0;
   texcoords[0][1]=0;
   vertices[0][0]=-100.f;
   vertices[0][1]=-100.f;
   vertices[0][2]=-320.f;

   texcoords[1][0]=1;
   texcoords[1][1]=0;
   vertices[1][0]=100.f;
   vertices[1][1]=-100.f;
   vertices[1][2]=-320.f;

   texcoords[3][0]=1;
   texcoords[3][1]=1;
   vertices[3][0]=100.f;
   vertices[3][1]=-100.f;
   vertices[3][2]=-520.f;

   texcoords[2][0]=0;
   texcoords[2][1]=1;
   vertices[2][0]=-100.f;
   vertices[2][1]=-100.f;
   vertices[2][2]=-520.f;

   glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

   glDisable(GL_TEXTURE_2D);

   glDisableClientState(GL_TEXTURE_COORD_ARRAY);
   glDisableClientState(GL_VERTEX_ARRAY);

   /* walls */

   glEnableClientState(GL_VERTEX_ARRAY);

   glNormal3f(1.f, 0.f, 0.f);
   vertices[0][0]=-100.f;
   vertices[0][1]=-100.f;
   vertices[0][2]=-320.f;
   vertices[1][0]=-100.f;
   vertices[1][1]=-100.f;
   vertices[1][2]=-520.f;
   vertices[3][0]=-100.f;
   vertices[3][1]=100.f;
   vertices[3][2]=-520.f;
   vertices[2][0]=-100.f;
   vertices[2][1]=100.f;
   vertices[2][2]=-320.f;
   glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

   glNormal3f(-1.f, 0.f, 0.f);
   vertices[0][0]=100.f;
   vertices[0][1]=-100.f;
   vertices[0][2]=-320.f;
   vertices[1][0]=100.f;
   vertices[1][1]=100.f;
   vertices[1][2]=-320.f;
   vertices[3][0]=100.f;
   vertices[3][1]=100.f;
   vertices[3][2]=-520.f;
   vertices[2][0]=100.f;
   vertices[2][1]=-100.f;
   vertices[2][2]=-520.f;
   glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

   glNormal3f(0.f, -1.f, 0.f);
   vertices[0][0]=-100.f;
   vertices[0][1]=100.f;
   vertices[0][2]=-320.f;
   vertices[1][0]=-100.f;
   vertices[1][1]=100.f;
   vertices[1][2]=-520.f;
   vertices[3][0]=100.f;
   vertices[3][1]=100.f;
   vertices[3][2]=-520.f;
   vertices[2][0]=100.f;
   vertices[2][1]=100.f;
   vertices[2][2]=-320.f;
   glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

   glNormal3f(0.f, 0.f, 1.f);
   vertices[0][0]=-100.f;
   vertices[0][1]=-100.f;
   vertices[0][2]=-520.f;
   vertices[1][0]=100.f;
   vertices[1][1]=-100.f;
   vertices[1][2]=-520.f;
   vertices[3][0]=100.f;
   vertices[3][1]=100.f;
   vertices[3][2]=-520.f;
   vertices[2][0]=-100.f;
   vertices[2][1]=100.f;
   vertices[2][2]=-520.f;
   glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

   glDisableClientState(GL_VERTEX_ARRAY);

   /* Draw smooth shaded cones */
   glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, cone_mat);

   glPushMatrix();
   glTranslatef(-50.f, 55.0f, -400.f);
   glRotatef(-90.f, 1.f, 0.f, 0.f);
   glRotatef(rotate, 1.f, 0.0f, 0.0f);
   gluCylinder(cone_fill_smooth, 20.0f, 10.0f, 40.0f, 30, 30);
   glPopMatrix();

   glPushMatrix();
   glTranslatef(0.0f, 55.0f, -400.f);
   glRotatef(-90.f, 1.f, 0.f, 0.f);
   glRotatef(rotate, 1.f, 0.0f, 0.0f);
   gluCylinder(cone_point_smooth, 20.0f, 10.0f, 40.0f, 60, 60);
   glPopMatrix();

   glPushMatrix();
   glTranslatef(50.f, 55.f, -400.f);
   glRotatef(-90.f, 1.f, 0.f, 0.f);
   glRotatef(rotate, 1.f, 0.0f, 0.0f);
   gluCylinder(cone_line_smooth, 20.0f, 10.0f, 40.0f, 40, 40);
   glPopMatrix();

   /* Draw flat shaded cones */
   glShadeModel(GL_FLAT);

   glPushMatrix();
   glTranslatef(-50.f, 0.0f, -400.f);
   glRotatef(-90.f, 1.f, 0.f, 0.f);
   glRotatef(rotate, 1.f, 0.0f, 0.0f);
   gluCylinder(cone_fill_flat, 20.0f, 10.0f, 40.0f, 30, 30);
   glPopMatrix();

   glPushMatrix();
   glTranslatef(0.0f, 0.0f, -400.f);
   glRotatef(-90.f, 1.f, 0.f, 0.f);
   glRotatef(rotate, 1.f, 0.0f, 0.0f);
   gluCylinder(cone_point_flat, 20.0f, 10.0f, 40.0f, 60, 60);
   glPopMatrix();

   glPushMatrix();
   glTranslatef(50.f, 0.f, -400.f);
   glRotatef(-90.f, 1.f, 0.f, 0.f);
   glRotatef(rotate, 1.f, 0.0f, 0.0f);
   gluCylinder(cone_line_flat, 20.0f, 10.0f, 40.0f, 40, 40);
   glPopMatrix();

   /* Draw textured cones */
   glShadeModel(GL_SMOOTH);
   glEnable(GL_TEXTURE_2D);

   glPushMatrix();
   glTranslatef(-50.f, -55.0f, -400.f);
   glRotatef(-90.f, 1.f, 0.f, 0.f);
   glRotatef(rotate, 1.f, 0.0f, 0.0f);
   gluCylinder(cone_fill_texture, 20.0f, 10.0f, 40.0f, 30, 30);
   glPopMatrix();

   glPushMatrix();
   glTranslatef(0.0f, -55.0f, -400.f);
   glRotatef(-90.f, 1.f, 0.f, 0.f);
   glRotatef(rotate, 1.f, 0.0f, 0.0f);
   gluCylinder(cone_point_texture, 20.0f, 10.0f, 40.0f, 60, 60);
   glPopMatrix();

   glPushMatrix();
   glTranslatef(50.f, -55.f, -400.f);
   glRotatef(-90.f, 1.f, 0.f, 0.f);
   glRotatef(rotate, 1.f, 0.0f, 0.0f);
   gluCylinder(cone_line_texture, 20.0f, 10.0f, 40.0f, 40, 40);
   glPopMatrix();

   rotate+=1.0f;

   if (glGetError())
   {
      printf("Oops! I screwed up my OpenGL ES calls somewhere\n");
   }
}

int TestEGLError(HWND hWnd, char* pszLocation)
{
   /*
      eglGetError returns the last error that has happened using egl,
      not the status of the last called function. The user has to
      check after every single egl call or at least once every frame.
   */

   EGLint iErr=eglGetError();
   if (iErr!=EGL_SUCCESS)
   {
      TCHAR pszStr[256];
      swprintf_s(pszStr, 256, _T("%s failed (%d).\n"), pszLocation, iErr);
      MessageBox(hWnd, pszStr, _T("Error"), MB_OK|MB_ICONEXCLAMATION);
      return 0;
   }

   return 1;
}

int done=0;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
   switch (message)
   {
      // Handles the close message when a user clicks the quit icon of the window
      case WM_CLOSE:
           done=1;
           PostQuitMessage(0);
           return 1;

      default:
	       break;
   }

   // Calls the default window procedure for messages we did not handle
   return DefWindowProc(hWnd, message, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR pCmdLine, int nCmdShow)
{
   int i;

   // Windows variables
   HWND hWnd=0;
   HDC  hDC=0;

   // EGL variables
   EGLDisplay eglDisplay=0;
   EGLConfig  eglConfig=0;
   EGLSurface eglSurface=0;
   EGLContext eglContext=0;
   NativeWindowType eglWindow=0;
   EGLint pi32ConfigAttribs[128];
   unsigned int nWidth;
   unsigned int nHeight;
   ATOM registerClass;
   RECT sRect;
   EGLint iMajorVersion, iMinorVersion;
   int iConfigs;

   /*
      Step 0 - Create a NativeWindowType that we can use for OpenGL ES output
   */

   // Register the windows class
   WNDCLASS sWC;

   sWC.style=CS_HREDRAW | CS_VREDRAW;
   sWC.lpfnWndProc=WndProc;
   sWC.cbClsExtra=0;
   sWC.cbWndExtra=0;
   sWC.hInstance=hInstance;
   sWC.hIcon=0;
   sWC.hCursor=0;
   sWC.lpszMenuName=0;
   sWC.hbrBackground=(HBRUSH)GetStockObject(WHITE_BRUSH);
   sWC.lpszClassName=WINDOW_CLASS;

   nWidth=WINDOW_WIDTH;
   nHeight=WINDOW_HEIGHT;

   registerClass=RegisterClass(&sWC);
   if (!registerClass)
   {
      MessageBox(0, _T("Failed to register the window class"), _T("Error"), MB_OK | MB_ICONEXCLAMATION);
   }

   // Create the eglWindow
   SetRect(&sRect, 0, 0, nWidth, nHeight);
   AdjustWindowRectEx(&sRect, WS_CAPTION | WS_SYSMENU, 0, 0);
   hWnd=CreateWindow(WINDOW_CLASS, _T("GLU ES Cylinder test"), WS_VISIBLE | WS_SYSMENU, 0, 0, nWidth, nHeight, NULL, NULL, hInstance, NULL);
   eglWindow=hWnd;

   // Get the associated device context
   hDC=GetDC(hWnd);
   if (!hDC)
   {
      MessageBox(0, _T("Failed to create the device context"), _T("Error"), MB_OK|MB_ICONEXCLAMATION);
      goto cleanup;
   }

   /*
      Step 1 - Get the default display.
      EGL uses the concept of a "display" which in most environments
      corresponds to a single physical screen. Since we usually want
      to draw to the main screen or only have a single screen to begin
      with, we let EGL pick the default display.
      Querying other displays is platform specific.
   */
   eglDisplay=eglGetDisplay((NativeDisplayType)hDC);

   if(eglDisplay==EGL_NO_DISPLAY)
   {
      eglDisplay=eglGetDisplay((NativeDisplayType)EGL_DEFAULT_DISPLAY);
   }

   /*
      Step 2 - Initialize EGL.
      EGL has to be initialized with the display obtained in the
      previous step. We cannot use other EGL functions except
      eglGetDisplay and eglGetError before eglInitialize has been
      called.
      If we're not interested in the EGL version number we can just
      pass NULL for the second and third parameters.
   */

   if (!eglInitialize(eglDisplay, &iMajorVersion, &iMinorVersion))
   {
      MessageBox(0, _T("eglInitialize() failed."), _T("Error"), MB_OK | MB_ICONEXCLAMATION);
      goto cleanup;
   }

   /*
      Step 3 - Specify the required configuration attributes.
      An EGL "configuration" describes the pixel format and type of
      surfaces that can be used for drawing.
      For now we just want to use a 16 bit RGB surface that is a
      Window surface, i.e. it will be visible on screen. The list
      has to contain key/value pairs, terminated with EGL_NONE.
   */
   i=0;

   pi32ConfigAttribs[i++]=EGL_RED_SIZE;
   pi32ConfigAttribs[i++]=5;
   pi32ConfigAttribs[i++]=EGL_GREEN_SIZE;
   pi32ConfigAttribs[i++]=6;
   pi32ConfigAttribs[i++]=EGL_BLUE_SIZE;
   pi32ConfigAttribs[i++]=5;
   pi32ConfigAttribs[i++]=EGL_ALPHA_SIZE;
   pi32ConfigAttribs[i++]=0;
   pi32ConfigAttribs[i++]=EGL_DEPTH_SIZE;
   pi32ConfigAttribs[i++]=16;
   pi32ConfigAttribs[i++]=EGL_SURFACE_TYPE;
   pi32ConfigAttribs[i++]=EGL_WINDOW_BIT;
   pi32ConfigAttribs[i++]=EGL_NONE;

   /*
      Step 4 - Find a config that matches all requirements.
      eglChooseConfig provides a list of all available configurations
      that meet or exceed the requirements given as the second
      argument. In most cases we just want the first config that meets
      all criteria, so we can limit the number of configs returned to 1.
   */
   if (!eglChooseConfig(eglDisplay, pi32ConfigAttribs, &eglConfig, 1, &iConfigs) || (iConfigs != 1))
   {
      MessageBox(0, _T("eglChooseConfig() failed."), _T("Error"), MB_OK|MB_ICONEXCLAMATION);
      goto cleanup;
   }

   /*
      Step 5 - Create a surface to draw to.
      Use the config picked in the previous step and the native window
      handle when available to create a window surface. A window surface
      is one that will be visible on screen inside the native display (or
      fullscreen if there is no windowing system).
      Pixmaps and pbuffers are surfaces which only exist in off-screen
      memory.
   */
   eglSurface=eglCreateWindowSurface(eglDisplay, eglConfig, eglWindow, NULL);

   if (eglSurface==EGL_NO_SURFACE)
   {
      eglGetError(); // Clear error
      eglSurface=eglCreateWindowSurface(eglDisplay, eglConfig, NULL, NULL);
   }

   if (!TestEGLError(hWnd, "eglCreateWindowSurface"))
   {
      goto cleanup;
   }

   /*
      Step 6 - Create a context.
      EGL has to create a context for OpenGL ES. Our OpenGL ES resources
      like textures will only be valid inside this context
      (or shared contexts)
   */
   eglContext=eglCreateContext(eglDisplay, eglConfig, NULL, NULL);
   if (!TestEGLError(hWnd, "eglCreateContext"))
   {
      goto cleanup;
   }

   /*
      Step 7 - Bind the context to the current thread and use our
      window surface for drawing and reading.
      Contexts are bound to a thread. This means you don't have to
      worry about other threads and processes interfering with your
      OpenGL ES application.
      We need to specify a surface that will be the target of all
      subsequent drawing operations, and one that will be the source
      of read operations. They can be the same surface.
   */
   eglMakeCurrent(eglDisplay, eglSurface, eglSurface, eglContext);
   if (!TestEGLError(hWnd, "eglMakeCurrent"))
   {
      goto cleanup;
   }

   /* Initialize scene */
   init_scene();

   /* Render stuff */
   do {
      MSG msg;

	  render_scene();
      glFinish();
      eglWaitGL();
      eglSwapBuffers(eglDisplay, eglSurface);
	  if (done)
	  {
         break;
	  }

      PeekMessage(&msg, hWnd, 0, 0, PM_REMOVE);
      TranslateMessage(&msg);
      DispatchMessage(&msg);

   } while(1);


   /*
      Step 8 - Terminate OpenGL ES and destroy the window (if present).
      eglTerminate takes care of destroying any context or surface created
      with this display, so we don't need to call eglDestroySurface or
      eglDestroyContext here.
   */
cleanup:
   eglMakeCurrent(eglDisplay, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
   eglTerminate(eglDisplay);

   /*
      Step 9 - Destroy the eglWindow.
      Again, this is platform specific and delegated to a separate function.
   */

   // Release the device context
   if (hDC)
   {
      ReleaseDC(hWnd, hDC);
   }

   // Destroy the eglWindow
   if (hWnd)
   {
      DestroyWindow(hWnd);
   }

   return 0;
}
