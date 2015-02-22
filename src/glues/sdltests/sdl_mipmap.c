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

GLfloat rotate=0;
GLUquadricObj* sphere;
GLUquadricObj* cone;

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

   if (height==0)
   {
      height=1;
   }

   /* Clear error */
   glGetError();

   /* Setup our viewport */
   glViewport(0, 0, (GLint)width, (GLint)height);

   /* draw a perspective scene */
   glMatrixMode(GL_PROJECTION);
   gluPerspective(35.0f, (GLfloat)width/(GLfloat)height, 320.0f, 6000.0f);

   glMatrixMode(GL_MODELVIEW);

   /* turn on features */
   glEnable(GL_DEPTH_TEST);
   glEnable(GL_LIGHTING);
   glEnable(GL_LIGHT0);

   /* place light 0 in the right place */
   glLightfv(GL_LIGHT0, GL_POSITION, lightpos);

   /* enable filtering */
   glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
   glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

   tex=make_texture(512, 512);
   gluBuild2DMipmaps(GL_TEXTURE_2D, GL_LUMINANCE, 512, 512, GL_LUMINANCE,
                     GL_UNSIGNED_BYTE, tex);
   free(tex);

   sphere=gluNewQuadric();
   gluQuadricDrawStyle(sphere, GLU_FILL);
   gluQuadricTexture(sphere, GLU_TRUE);

   cone=gluNewQuadric();
   gluQuadricDrawStyle(cone, GLU_FILL);
   gluQuadricTexture(cone, GLU_TRUE);

   if (glGetError())
   {
      printf("Oops! I screwed up my OpenGL ES calls somewhere\n");
   }
}

void render_scene()
{
   /* material properties for objects in scene */
   static GLfloat wall_mat[4]={1.0f, 1.0f, 1.0f, 1.0f};
   static GLfloat sphere_mat[4]={1.0f, 0.7f, 0.2f, 1.0f};
   static GLfloat sphere2_mat[4]={0.2f, 0.7f, 0.2f, 1.0f};
   static GLfloat sphere3_mat[4]={0.2f, 0.2f, 0.7f, 1.0f};
   static GLfloat cone_mat[4]={1.0f, 0.2f, 0.2f, 1.0f};
   GLfloat texcoords[4][2];
   GLfloat vertices[4][3];

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

   /* Set up global normal */
   glNormal3f(0.f, 1.f, 0.f);

   /* Fill texture coordinates and vertices arrays */
   texcoords[0][0]=0; texcoords[0][1]=0;
   vertices[0][0]=-200.f; vertices[0][1]=-100.f; vertices[0][2]=-320.f;
   texcoords[1][0]=1; texcoords[1][1]=0;
   vertices[1][0]=200.f; vertices[1][1]=-100.f; vertices[1][2]=-320.f;
   texcoords[3][0]=1; texcoords[3][1]=1;
   vertices[3][0]=200.f; vertices[3][1]=400.f; vertices[3][2]=-2000.f;
   texcoords[2][0]=0; texcoords[2][1]=1;
   vertices[2][0]=-200.f; vertices[2][1]=400.f; vertices[2][2]=-2000.f;
   glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

   /* Draw Sphere */
   glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, sphere_mat);
   glPushMatrix();
   glTranslatef(0.0f, 0.0f, -400.f);
   glRotatef(-90.f, 1.f, 0.f, 0.f);
   glRotatef(rotate, 1.f, 0.0f, 0.0f);
   gluSphere(sphere, 24.0f, 30, 30);
   glPopMatrix();

   /* Draw Sphere 2 */
   glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, sphere2_mat);
   glPushMatrix();
   glTranslatef(-50.0f, 50.0f, -550.f);
   glRotatef(-90.f, 0.f, 1.f, 0.f);
   glRotatef(rotate, 1.f, 0.0f, 0.0f);
   gluSphere(sphere, 24.0f, 30, 30);
   glPopMatrix();

   /* Draw Sphere 3 */
   glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, sphere3_mat);
   glPushMatrix();
   glTranslatef(50.0f, 100.0f, -700.f);
   glRotatef(-90.f, 0.f, 0.f, 1.f);
   glRotatef(rotate, 1.f, 0.0f, 0.0f);
   gluSphere(sphere, 24.0f, 30, 30);
   glPopMatrix();

   /* Draw Cone */
   glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE, cone_mat);
   glPushMatrix();
   glTranslatef(0.0f, 150.0f, -850.f);
   glRotatef(-90.f, 1.f, 0.f, 0.f);
   glRotatef(rotate, 0.0f, 0.0f, 1.0f);
   gluCylinder(cone, 20.0f, 0.0f, 60.0f, 40, 40);
   glPopMatrix();

   rotate+=1.0f;

   glDisable(GL_TEXTURE_2D);
   glDisableClientState(GL_TEXTURE_COORD_ARRAY);
   glDisableClientState(GL_VERTEX_ARRAY);

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

   window=SDL_CreateWindow("SDL GLU ES MipMap test",
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
