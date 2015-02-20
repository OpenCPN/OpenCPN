/*                                                              */
/* This test is based on tess.c from OpenGL Redbook,            */
/* initialization part on QSSL's egl* demo                      */
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

#define TESS_OBJ_MAX_SEQS      5
#define TESS_OBJ_MAX_VERTICES  32

typedef struct TesselatedObject
{
   GLuint  sequences;
   GLenum  seqtype[TESS_OBJ_MAX_SEQS];
   GLuint  verticesperseq[TESS_OBJ_MAX_SEQS];
   GLfloat vertices[TESS_OBJ_MAX_SEQS][TESS_OBJ_MAX_VERTICES][3];
   GLfloat colors[TESS_OBJ_MAX_SEQS][TESS_OBJ_MAX_VERTICES][4];
   GLfloat texcoords[TESS_OBJ_MAX_SEQS][TESS_OBJ_MAX_VERTICES][2];
} TesselatedObject;

/* rectangle with triangular hole inside */
TesselatedObject object1;
/* self-intersecting star */
TesselatedObject object2;
/* triangle with triangular holes inside */
TesselatedObject object3;

GLvoid beginCallback(GLenum which, void* polygondata)
{
   TesselatedObject* object=(TesselatedObject*)polygondata;

   /* Check for available space in the structure */
   if (object->sequences>=TESS_OBJ_MAX_SEQS)
   {
      fprintf(stderr, "Error, there is no free space in TesselationObject\n");
      return;
   }

   /* Store primitive sequence type */
   object->seqtype[object->sequences]=which;
   object->verticesperseq[object->sequences]=0;
}

GLvoid errorCallback(GLenum errorCode)
{
   const GLubyte* estring;

   estring=gluErrorString(errorCode);
   fprintf(stderr, "Tessellation Error: %s\n", estring);
   exit(0);
}

GLvoid endCallback(void* polygondata)
{
   TesselatedObject* object=(TesselatedObject*)polygondata;

   /* Check for available space in the structure */
   if (object->sequences>=TESS_OBJ_MAX_SEQS)
   {
      fprintf(stderr, "Error, there is no free space in TesselationObject\n");
      return;
   }

   /* Increase count of primitive sequences */
   object->sequences++;
}

void vertexCallback(GLvoid* vertex, void* polygondata)
{
   TesselatedObject* object=(TesselatedObject*)polygondata;
   const GLfloat* pointer;

   /* Check for available space in the structure */
   if (object->sequences>=TESS_OBJ_MAX_SEQS)
   {
      fprintf(stderr, "Error, there is no free space in TesselationObject\n");
      return;
   }

   if (object->verticesperseq[object->sequences]>=TESS_OBJ_MAX_VERTICES)
   {
      fprintf(stderr, "Error, there is no free vertex space in TesselationObject\n");
      return;
   }

   pointer=(GLfloat*)vertex;

   object->vertices[object->sequences][object->verticesperseq[object->sequences]][0]=pointer[0];
   object->vertices[object->sequences][object->verticesperseq[object->sequences]][1]=pointer[1];
   object->vertices[object->sequences][object->verticesperseq[object->sequences]][2]=pointer[2];

   /* Increase amount of vertices in current primitive sequence */
   object->verticesperseq[object->sequences]++;
}

void vertexColorCallback(GLvoid* vertex, void* polygondata)
{
   TesselatedObject* object=(TesselatedObject*)polygondata;
   const GLfloat* pointer;

   /* Check for available space in the structure */
   if (object->sequences>=TESS_OBJ_MAX_SEQS)
   {
      fprintf(stderr, "Error, there is no free space in TesselationObject\n");
      return;
   }

   if (object->verticesperseq[object->sequences]>=TESS_OBJ_MAX_VERTICES)
   {
      fprintf(stderr, "Error, there is no free vertex space in TesselationObject\n");
      return;
   }

   pointer=(GLfloat*)vertex;

   object->vertices[object->sequences][object->verticesperseq[object->sequences]][0]=pointer[0];
   object->vertices[object->sequences][object->verticesperseq[object->sequences]][1]=pointer[1];
   object->vertices[object->sequences][object->verticesperseq[object->sequences]][2]=pointer[2];

   object->colors[object->sequences][object->verticesperseq[object->sequences]][0]=pointer[3];
   object->colors[object->sequences][object->verticesperseq[object->sequences]][1]=pointer[4];
   object->colors[object->sequences][object->verticesperseq[object->sequences]][2]=pointer[5];
   object->colors[object->sequences][object->verticesperseq[object->sequences]][3]=1.0f;

   /* Increase amount of vertices in current primitive sequence */
   object->verticesperseq[object->sequences]++;
}

void vertexTexColorCallback(GLvoid* vertex, void* polygondata)
{
   TesselatedObject* object=(TesselatedObject*)polygondata;
   const GLfloat* pointer;

   /* Check for available space in the structure */
   if (object->sequences>=TESS_OBJ_MAX_SEQS)
   {
      fprintf(stderr, "Error, there is no free space in TesselationObject\n");
      return;
   }

   if (object->verticesperseq[object->sequences]>=TESS_OBJ_MAX_VERTICES)
   {
      fprintf(stderr, "Error, there is no free vertex space in TesselationObject\n");
      return;
   }

   pointer=(GLfloat*)vertex;

   object->vertices[object->sequences][object->verticesperseq[object->sequences]][0]=pointer[0];
   object->vertices[object->sequences][object->verticesperseq[object->sequences]][1]=pointer[1];
   object->vertices[object->sequences][object->verticesperseq[object->sequences]][2]=pointer[2];

   object->colors[object->sequences][object->verticesperseq[object->sequences]][0]=pointer[3];
   object->colors[object->sequences][object->verticesperseq[object->sequences]][1]=pointer[4];
   object->colors[object->sequences][object->verticesperseq[object->sequences]][2]=pointer[5];
   object->colors[object->sequences][object->verticesperseq[object->sequences]][3]=1.0f;

   object->texcoords[object->sequences][object->verticesperseq[object->sequences]][0]=pointer[6];
   object->texcoords[object->sequences][object->verticesperseq[object->sequences]][1]=pointer[7];

   /* Increase amount of vertices in current primitive sequence */
   object->verticesperseq[object->sequences]++;
}

void combineColorCallback(GLfloat coords[3], GLfloat* vertex_data[4],
                          GLfloat weight[4], GLfloat** dataOut)
{
   GLfloat* vertex;
   int i;

   vertex=(GLfloat*)malloc(6*sizeof(GLfloat));
   vertex[0]=coords[0];
   vertex[1]=coords[1];
   vertex[2]=coords[2];
   for (i=3; i<6; i++)
   {
      vertex[i]=weight[0]*vertex_data[0][i] +
                weight[1]*vertex_data[1][i] +
                weight[2]*vertex_data[2][i] +
                weight[3]*vertex_data[3][i];
   }
   *dataOut=vertex;
}

void combineTexColorCallback(GLfloat coords[3], GLfloat* vertex_data[4],
                             GLfloat weight[4], GLfloat** dataOut)
{
   GLfloat* vertex;
   int i;

   vertex=(GLfloat*)malloc(8*sizeof(GLfloat));
   vertex[0]=coords[0];
   vertex[1]=coords[1];
   vertex[2]=coords[2];
   for (i=3; i<6; i++)
   {
      vertex[i]=weight[0]*vertex_data[0][i] +
                weight[1]*vertex_data[1][i] +
                weight[2]*vertex_data[2][i] +
                weight[3]*vertex_data[3][i];
   }
   for (i=6; i<8; i++)
   {
      vertex[i]=weight[0]*vertex_data[0][i] +
                weight[1]*vertex_data[1][i] +
                weight[2]*vertex_data[2][i] +
                weight[3]*vertex_data[3][i];
   }
   *dataOut=vertex;
}

/* Create a single component texture map */
GLubyte* make_texture(int maxs, int maxt)
{
   int s, t;
   static GLubyte* texture;

   texture=(GLubyte*)malloc(maxs*maxt*sizeof(GLubyte));
   for (t=0; t<maxt; t++)
   {
      for (s=0; s<maxs; s++)
      {
         texture[s+maxs*t]=(((s>>4)&0x1)^((t>>4)&0x1))*255;
         if (texture[s+maxs*t]==0)
         {
            texture[s+maxs*t]=128;
         }
      }
   }

   return texture;
}

void init_scene(int width, int height)
{
   GLubyte* tex;

   GLUtesselator* tobj;
   GLfloat rect[4][3]={
                         {50.0f,  50.0f,  0.0f},
                         {200.0f, 50.0f,  0.0f},
                         {200.0f, 200.0f, 0.0f},
                         {50.0f,  200.0f, 0.0f}
                      };
   GLfloat tri[3][3]={
                        {75.0f,  75.0f,  0.0f},
                        {125.0f, 175.0f, 0.0f},
                        {175.0f, 75.0f,  0.0f}
                     };
   GLfloat star[5][6]={
                         {250.0f, 50.0f,  0.0f, 1.0f, 0.0f, 1.0f},
                         {325.0f, 200.0f, 0.0f, 1.0f, 1.0f, 0.0f},
                         {400.0f, 50.0f,  0.0f, 0.0f, 1.0f, 1.0f},
                         {250.0f, 150.0f, 0.0f, 1.0f, 0.0f, 0.0f},
                         {400.0f, 150.0f, 0.0f, 0.0f, 1.0f, 0.0f}
                      };
   GLfloat triangle[9][8]={
                             {450.0f, 50.0f,  0.0f, 1.0f,  0.0f,  0.0f,  0.0f,  0.0f},
                             {525.0f, 200.0f, 0.0f, 1.0f,  1.0f,  1.0f,  0.5f,  1.0f},
                             {600.0f, 50.0f,  0.0f, 0.0f,  0.0f,  1.0f,  1.0f,  0.0f},
                             {500.0f, 50.0f,  0.0f, 0.666f, 0.000f, 0.333f, 0.333f, 0.000f},
                             {550.0f, 150.0f, 0.0f, 0.666f, 0.666f, 1.000f, 0.666f, 0.666f},
                             {500.0f, 150.0f, 0.0f, 1.000f, 0.666f, 0.666f, 0.333f, 0.666f},
                             {550.0f, 50.0f,  0.0f, 0.333f, 0.000f, 0.666f, 0.666f, 0.000f},
                             {575.0f, 100.0f, 0.0f, 0.333f, 0.333f, 1.000f, 0.825f, 0.333f},
                             {475.0f, 100.0f, 0.0f, 1.000f, 0.333f, 0.333f, 0.175f, 0.333f}
                          };

   glViewport(0, 0, (GLsizei)width, (GLsizei)height);
   glMatrixMode(GL_PROJECTION);
   glLoadIdentity();
   gluOrtho2D(0.0f, (GLfloat)width, 0.0f, (GLfloat)height);

   glClearColor(0.0, 0.0, 0.0, 0.0);

   /* Create new triangulator */
   tobj=gluNewTess();

   /* Set triangulator's callbacks */
   gluTessCallback(tobj, GLU_TESS_VERTEX_DATA, (GLvoid (*)())&vertexCallback);
   gluTessCallback(tobj, GLU_TESS_BEGIN_DATA, (GLvoid (*)())&beginCallback);
   gluTessCallback(tobj, GLU_TESS_END_DATA, (GLvoid (*)())&endCallback);
   gluTessCallback(tobj, GLU_TESS_ERROR, (GLvoid (*)())&errorCallback);

   /* reset object */
   object1.sequences=0;

   /* rectangle with triangular hole inside */
   gluTessBeginPolygon(tobj, (void*)&object1);
      gluTessBeginContour(tobj);
         gluTessVertex(tobj, rect[0], rect[0]);
         gluTessVertex(tobj, rect[1], rect[1]);
         gluTessVertex(tobj, rect[2], rect[2]);
         gluTessVertex(tobj, rect[3], rect[3]);
      gluTessEndContour(tobj);
      gluTessBeginContour(tobj);
         gluTessVertex(tobj, tri[0], tri[0]);
         gluTessVertex(tobj, tri[1], tri[1]);
         gluTessVertex(tobj, tri[2], tri[2]);
      gluTessEndContour(tobj);
   gluTessEndPolygon(tobj);

   /* Set triangulator's callbacks */
   gluTessCallback(tobj, GLU_TESS_VERTEX_DATA, (GLvoid (*)())&vertexColorCallback);
   gluTessCallback(tobj, GLU_TESS_BEGIN_DATA, (GLvoid (*)())&beginCallback);
   gluTessCallback(tobj, GLU_TESS_END_DATA, (GLvoid (*)())&endCallback);
   gluTessCallback(tobj, GLU_TESS_ERROR, (GLvoid (*)())&errorCallback);
   gluTessCallback(tobj, GLU_TESS_COMBINE, (GLvoid (*)())&combineColorCallback);

   /* reset object */
   object2.sequences=0;

   /*  self-intersecting star  */
   gluTessProperty(tobj, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_POSITIVE);
   gluTessBeginPolygon(tobj, (void*)&object2);
      gluTessBeginContour(tobj);
         gluTessVertex(tobj, star[0], star[0]);
         gluTessVertex(tobj, star[1], star[1]);
         gluTessVertex(tobj, star[2], star[2]);
         gluTessVertex(tobj, star[3], star[3]);
         gluTessVertex(tobj, star[4], star[4]);
      gluTessEndContour(tobj);
   gluTessEndPolygon(tobj);

   /* Set triangulator's callbacks */
   gluTessCallback(tobj, GLU_TESS_VERTEX_DATA, (GLvoid (*)())&vertexTexColorCallback);
   gluTessCallback(tobj, GLU_TESS_BEGIN_DATA, (GLvoid (*)())&beginCallback);
   gluTessCallback(tobj, GLU_TESS_END_DATA, (GLvoid (*)())&endCallback);
   gluTessCallback(tobj, GLU_TESS_ERROR, (GLvoid (*)())&errorCallback);
   gluTessCallback(tobj, GLU_TESS_COMBINE, (GLvoid (*)())&combineTexColorCallback);

   /* reset object */
   object3.sequences=0;

   /*  self-intersecting star  */
   gluTessProperty(tobj, GLU_TESS_WINDING_RULE, GLU_TESS_WINDING_POSITIVE);
   gluTessBeginPolygon(tobj, (void*)&object3);
      gluTessBeginContour(tobj);
         gluTessVertex(tobj, triangle[0], triangle[0]);
         gluTessVertex(tobj, triangle[1], triangle[1]);
         gluTessVertex(tobj, triangle[2], triangle[2]);
      gluTessEndContour(tobj);
      gluTessBeginContour(tobj);
         gluTessVertex(tobj, triangle[3], triangle[3]);
         gluTessVertex(tobj, triangle[4], triangle[4]);
         gluTessVertex(tobj, triangle[5], triangle[5]);
         gluTessVertex(tobj, triangle[6], triangle[6]);
         gluTessVertex(tobj, triangle[7], triangle[7]);
         gluTessVertex(tobj, triangle[8], triangle[8]);
      gluTessEndContour(tobj);
   gluTessEndPolygon(tobj);

   /* Destroy triangulator object */
   gluDeleteTess(tobj);

   /* enable filtering */
   glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
   glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

   tex=make_texture(256, 256);
   glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, 256, 256, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE, tex);
   free(tex);
}

void render_scene()
{
   unsigned int it;

   glClear(GL_COLOR_BUFFER_BIT);
   glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

   /* draw first object */
   glShadeModel(GL_FLAT);

   /* enable vertex array */
   glEnableClientState(GL_VERTEX_ARRAY);

   /* Emit first object */
   for (it=0; it<object1.sequences; it++)
   {
      /* Setup vertex pointer to the current sequence */
      glVertexPointer(3, GL_FLOAT, 0, &object1.vertices[it]);
      /* Draw the sequence */
      glDrawArrays(object1.seqtype[it], 0, object1.verticesperseq[it]);
   }

   /* disable vertex array */
   glDisableClientState(GL_VERTEX_ARRAY);

   /* draw second object */
   glShadeModel(GL_SMOOTH);

   /* enable vertex and color arrays */
   glEnableClientState(GL_VERTEX_ARRAY);
   glEnableClientState(GL_COLOR_ARRAY);

   /* Emit second object */
   for (it=0; it<object2.sequences; it++)
   {
      /* Setup vertex pointer to the current sequence */
      glVertexPointer(3, GL_FLOAT, 0, &object2.vertices[it]);
      /* Setup color pointer to the current sequence */
      glColorPointer(4, GL_FLOAT, 0, &object2.colors[it]);
      /* Draw the sequence */
      glDrawArrays(object2.seqtype[it], 0, object2.verticesperseq[it]);
   }

   /* disable vertex and color arrays */
   glDisableClientState(GL_VERTEX_ARRAY);
   glDisableClientState(GL_COLOR_ARRAY);

   /* enable vertex, texcoord and color arrays */
   glEnable(GL_TEXTURE_2D);
   glEnableClientState(GL_VERTEX_ARRAY);
   glEnableClientState(GL_COLOR_ARRAY);
   glEnableClientState(GL_TEXTURE_COORD_ARRAY);

   /* Emit third object */
   for (it=0; it<object3.sequences; it++)
   {
      /* Setup vertex pointer to the current sequence */
      glVertexPointer(3, GL_FLOAT, 0, &object3.vertices[it]);
      /* Setup color pointer to the current sequence */
      glColorPointer(4, GL_FLOAT, 0, &object3.colors[it]);
      /* Setup texture coordinates pointer to the current sequence */
      glTexCoordPointer(2, GL_FLOAT, 0, &object3.texcoords[it]);
      /* Draw the sequence */
      glDrawArrays(object3.seqtype[it], 0, object3.verticesperseq[it]);
   }

   /* disable vertex and color arrays */
   glDisableClientState(GL_VERTEX_ARRAY);
   glDisableClientState(GL_COLOR_ARRAY);
   glDisableClientState(GL_TEXTURE_COORD_ARRAY);
   glDisable(GL_TEXTURE_2D);

   /* Flush all drawings */
   glFlush();
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
   init_scene(WINDOW_WIDTH, WINDOW_HEIGHT);

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
