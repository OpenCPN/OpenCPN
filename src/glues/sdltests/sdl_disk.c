/*                                                              */
/* This test is based on accumaa.c - by Tom McReynolds, SGI     */
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

GLUquadricObj* disk_fill;
GLUquadricObj* disk_fill_flat;
GLUquadricObj* disk_fill_texture;
GLUquadricObj* disk_point;
GLUquadricObj* disk_line;
GLUquadricObj* disk_silh;
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

void init_scene(int width, int height)
{
   static GLfloat lightpos[4]={50.0f, 50.0f, -320.f, 1.0f};
   GLubyte* tex;

   /* Clear error */
   glGetError();

   /* Setup our viewport */
   glViewport(0, 0, (GLint)width, (GLint)height);

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

   disk_fill=gluNewQuadric();
   gluQuadricDrawStyle(disk_fill, GLU_FILL);
   gluQuadricNormals(disk_fill, GLU_SMOOTH);

   disk_fill_flat=gluNewQuadric();
   gluQuadricDrawStyle(disk_fill_flat, GLU_FILL);
   gluQuadricNormals(disk_fill_flat, GLU_FLAT);

   disk_fill_texture=gluNewQuadric();
   gluQuadricDrawStyle(disk_fill_texture, GLU_FILL);
   gluQuadricNormals(disk_fill_texture, GLU_SMOOTH);
   gluQuadricTexture(disk_fill_texture, GLU_TRUE);

   disk_point=gluNewQuadric();
   gluQuadricDrawStyle(disk_point, GLU_POINT);
   gluQuadricNormals(disk_point, GLU_SMOOTH);

   disk_line=gluNewQuadric();
   gluQuadricDrawStyle(disk_line, GLU_LINE);
   gluQuadricNormals(disk_line, GLU_SMOOTH);

   disk_silh=gluNewQuadric();
   gluQuadricDrawStyle(disk_silh, GLU_SILHOUETTE);

   if (glGetError())
   {
      printf("Oops! I screwed up my OpenGL ES calls somewhere\n");
   }
}

void render_scene()
{
   /* material properties for objects in scene */
   static GLfloat wall_mat[4]={1.0f, 1.0f, 1.0f, 1.0f};
   static GLfloat disk_mat[4]={0.5f, 0.5f, 1.0f, 1.0f};
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
   texcoords[0][0]=0; texcoords[0][1]=0;
   vertices[0][0]=-100.f; vertices[0][1]=-100.f; vertices[0][2]=-320.f;
   texcoords[1][0]=1; texcoords[1][1]=0;
   vertices[1][0]=100.f; vertices[1][1]=-100.f; vertices[1][2]=-320.f;
   texcoords[3][0]=1; texcoords[3][1]=1;
   vertices[3][0]=100.f; vertices[3][1]=-100.f; vertices[3][2]=-520.f;
   texcoords[2][0]=0;  texcoords[2][1]=1;
   vertices[2][0]=-100.f; vertices[2][1]=-100.f; vertices[2][2]=-520.f;
   glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

   glDisable(GL_TEXTURE_2D);

   glDisableClientState(GL_TEXTURE_COORD_ARRAY);
   glDisableClientState(GL_VERTEX_ARRAY);

   /* walls */

   glEnableClientState(GL_VERTEX_ARRAY);

   glNormal3f(1.f, 0.f, 0.f);
   vertices[0][0]=-100.f; vertices[0][1]=-100.f; vertices[0][2]=-320.f;
   vertices[1][0]=-100.f; vertices[1][1]=-100.f; vertices[1][2]=-520.f;
   vertices[3][0]=-100.f; vertices[3][1]=100.f;  vertices[3][2]=-520.f;
   vertices[2][0]=-100.f; vertices[2][1]=100.f;  vertices[2][2]=-320.f;
   glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

   glNormal3f(-1.f, 0.f, 0.f);
   vertices[0][0]=100.f; vertices[0][1]=-100.f; vertices[0][2]=-320.f;
   vertices[1][0]=100.f; vertices[1][1]=100.f;  vertices[1][2]=-320.f;
   vertices[3][0]=100.f; vertices[3][1]=100.f;  vertices[3][2]=-520.f;
   vertices[2][0]=100.f; vertices[2][1]=-100.f; vertices[2][2]=-520.f;
   glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

   glNormal3f(0.f, -1.f, 0.f);
   vertices[0][0]=-100.f; vertices[0][1]=100.f; vertices[0][2]=-320.f;
   vertices[1][0]=-100.f; vertices[1][1]=100.f; vertices[1][2]=-520.f;
   vertices[3][0]=100.f;  vertices[3][1]=100.f; vertices[3][2]=-520.f;
   vertices[2][0]=100.f;  vertices[2][1]=100.f; vertices[2][2]=-320.f;
   glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

   glNormal3f(0.f, 0.f, 1.f);
   vertices[0][0]=-100.f; vertices[0][1]=-100.f; vertices[0][2]=-520.f;
   vertices[1][0]=100.f;  vertices[1][1]=-100.f; vertices[1][2]=-520.f;
   vertices[3][0]=100.f;  vertices[3][1]=100.f;  vertices[3][2]=-520.f;
   vertices[2][0]=-100.f; vertices[2][1]=100.f;  vertices[2][2]=-520.f;
   glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

   glDisableClientState(GL_VERTEX_ARRAY);

   /* Draw disk */
   glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, disk_mat);

   glPushMatrix();
   glTranslatef(-50.f, 0.0f, -400.f);
   glRotatef(-90.f, 1.f, 0.f, 0.f);
   glRotatef(rotate, 1.f, 0.0f, 0.0f);
   gluDisk(disk_fill, 5.0f, 24.0f, 30, 1);
   glPopMatrix();

   glPushMatrix();
   glTranslatef(0.0f, 0.0f, -400.f);
   glRotatef(-90.f, 1.f, 0.f, 0.f);
   glRotatef(rotate, 1.f, 0.0f, 0.0f);
   gluDisk(disk_point, 5.0f, 24.0f, 60, 5);
   glPopMatrix();

   glPushMatrix();
   glTranslatef(50.f, 0.f, -400.f);
   glRotatef(-90.f, 1.f, 0.f, 0.f);
   glRotatef(rotate, 1.f, 0.0f, 0.0f);
   gluDisk(disk_line, 5.0f, 24.0f, 40, 5);
   glPopMatrix();

   glPushMatrix();
   glTranslatef(0.0f, 50.0f, -400.f);
   glRotatef(-90.f, 1.f, 0.f, 0.f);
   glRotatef(rotate, 1.f, 0.0f, 0.0f);
   gluDisk(disk_silh, 5.0f, 24.0f, 40, 5);
   glPopMatrix();

   /* Draw flat shaded disk */
   glShadeModel(GL_FLAT);

   glPushMatrix();
   glTranslatef(-50.f, 50.0f, -400.f);
   glRotatef(-90.f, 1.f, 0.f, 0.f);
   glRotatef(rotate, 1.f, 0.0f, 0.0f);
   gluDisk(disk_fill_flat, 5.0f, 24.0f, 30, 1);
   glPopMatrix();

   /* Draw textured disk */
   glEnable(GL_TEXTURE_2D);
   glShadeModel(GL_SMOOTH);

   glPushMatrix();
   glTranslatef(-50.f, -50.0f, -400.f);
   glRotatef(-90.f, 1.f, 0.f, 0.f);
   glRotatef(rotate, 1.f, 0.0f, 0.0f);
   gluDisk(disk_fill_texture, 5.0f, 24.0f, 30, 1);
   glPopMatrix();

   glPushMatrix();
   glTranslatef(0.f, -50.0f, -400.f);
   glRotatef(-90.f, 1.f, 0.f, 0.f);
   glRotatef(rotate, 1.f, 0.0f, 0.0f);
   gluDisk(disk_fill_texture, 0.0f, 24.0f, 30, 1);
   glPopMatrix();

   rotate+=1.0f;

   if (glGetError())
   {
      printf("Oops! I screwed up my OpenGL ES calls somewhere\n");
   }

   glFinish();
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

   window=SDL_CreateWindow("SDL GLU ES Disk test",
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
