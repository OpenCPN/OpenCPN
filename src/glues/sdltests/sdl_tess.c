/*                                                              */
/* This test is based on tess.c from OpenGL Redbook.            */
/*                                                              */
/* // Mike Gorchak, 2009. GLU ES test                           */
/*                                                              */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#include <SDL/SDL.h>
#include <SDL/SDL_opengles.h>

#define __USE_SDL_GLES__
#include "glues.h"

/* screen width, height, and bit depth */
#define WINDOW_WIDTH  640
#define WINDOW_HEIGHT 480

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
   uint32_t it;

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

int main(int argc, char** argv)
{
   int status;
   SDL_WindowID window;
   SDL_GLContext glcontext=NULL;
   SDL_Event event;
   SDL_bool done=SDL_FALSE;

   status=SDL_Init(SDL_INIT_VIDEO);
   if (status<0)
   {
      fprintf(stderr, "Can't init default SDL video driver: %s\n", SDL_GetError());
      exit(-1);
   }

   /* Select first display */
   status=SDL_SelectVideoDisplay(0);
   if (status<0)
   {
      fprintf(stderr, "Can't attach to first display: %s\n", SDL_GetError());
      exit(-1);
   }

   window=SDL_CreateWindow("SDL GLU ES Tesselation test",
      SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
      WINDOW_WIDTH, WINDOW_HEIGHT,
      SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
   if (window==0)
   {
      fprintf(stderr, "Can't create window: %s\n", SDL_GetError());
      exit(-1);
   }

   glcontext=SDL_GL_CreateContext(window);
   if (glcontext==NULL)
   {
      fprintf(stderr, "Can't create OpenGL ES context: %s\n", SDL_GetError());
      exit(-1);
   }

   status=SDL_GL_MakeCurrent(window, glcontext);
   if (status<0)
   {
      fprintf(stderr, "Can't set current OpenGL ES context: %s\n", SDL_GetError());
      exit(-1);
   }

   init_scene(WINDOW_WIDTH, WINDOW_HEIGHT);

   do {
      /* handle the events in the queue */
      while (SDL_PollEvent(&event))
      {
         switch(event.type)
         {
            case SDL_WINDOWEVENT:
                 switch (event.window.event)
                 {
                    case SDL_WINDOWEVENT_CLOSE:
                         done=SDL_TRUE;
                         break;
                 }
                 break;
            case SDL_KEYDOWN:
                 switch (event.key.keysym.sym)
                 {
                    case SDLK_ESCAPE:
                         done=SDL_TRUE;
                         break;
                 }
                 break;
            case SDL_QUIT:
                 done=SDL_TRUE;
                 break;
         }
      }

      if (done==SDL_TRUE)
      {
         break;
      }

      render_scene();
      SDL_GL_SwapWindow(window);
   } while(1);

   SDL_GL_DeleteContext(glcontext);
   SDL_DestroyWindow(window);
   SDL_Quit();

   return 0;
}
