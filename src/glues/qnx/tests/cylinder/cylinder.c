/*                                                              */
/* This test is based on accumaa.c - by Tom McReynolds, SGI and */
/* initialization part on QSSL's egl* demo                      */
/*                                                              */
/* // Mike Gorchak, 2009. GLU ES test                           */
/*                                                              */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>

#include <gf/gf.h>
#include <gf/gf3d.h>
#include <GLES/gl.h>
#include <GLES/egl.h>

#include "glues.h"

gf_dev_t    gfdev;
gf_layer_t  layer;
int         layer_idx;

static EGLDisplay display;
static EGLSurface surface;

static EGLint attribute_list[]=
{
   EGL_NATIVE_VISUAL_ID, 0,
   EGL_NATIVE_RENDERABLE, EGL_TRUE,
   EGL_RED_SIZE, 5,
   EGL_GREEN_SIZE, 5,
   EGL_BLUE_SIZE, 5,
   EGL_DEPTH_SIZE, 16,
   EGL_NONE
};

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

int main(int argc, char** argv)
{
   gf_3d_target_t      target;
   gf_display_t        gf_disp;
   EGLConfig           config;
   EGLContext          econtext;
   EGLint              num_config;
   gf_dev_info_t       info;
   gf_layer_info_t     linfo;
   gf_display_info_t   disp_info;
   GLuint              width, height;
   GLuint              it;

   /* initialize the graphics device */
   if (gf_dev_attach(&gfdev, NULL, &info)!=GF_ERR_OK)
   {
      perror("gf_dev_attach()");
      return -1;
   }

   /* Setup the layer we will use */
   if (gf_display_attach(&gf_disp, gfdev, 0, &disp_info)!=GF_ERR_OK)
   {
      fprintf(stderr, "gf_display_attach() failed\n");
      return -1;
   }

   layer_idx=disp_info.main_layer_index;

   /* get an EGL display connection */
   display=eglGetDisplay(gfdev);
   if (display==EGL_NO_DISPLAY)
   {
      fprintf(stderr, "eglGetDisplay() failed\n");
      return -1;
   }

   width=disp_info.xres;
   height=disp_info.yres;

   if (gf_layer_attach(&layer, gf_disp, layer_idx, 0)!=GF_ERR_OK)
   {
      fprintf(stderr, "gf_layer_attach() failed\n");
      return -1;
   }

   /* initialize the EGL display connection */
   if (eglInitialize(display, NULL, NULL)!=EGL_TRUE)
   {
      fprintf(stderr, "eglInitialize: error 0x%x\n", eglGetError());
      return -1;
   }

   for (it=0;; it++)
   {
      /* Walk through all possible pixel formats for this layer */
      if (gf_layer_query(layer, it, &linfo)==-1)
      {
         fprintf(stderr, "Couldn't find a compatible frame "
                         "buffer configuration on layer %d\n", layer_idx);
         return -1;
      }

      /*
       * We want the color buffer format to match the layer format,
       * so request the layer format through EGL_NATIVE_VISUAL_ID.
       */
      attribute_list[1]=linfo.format;

      /* Look for a compatible EGL frame buffer configuration */
      if (eglChooseConfig(display, attribute_list, &config, 1, &num_config)==EGL_TRUE)
      {
         if (num_config>0)
         {
            break;
         }
      }
   }

   /* create a 3D rendering target */
   if (gf_3d_target_create(&target, layer, NULL, 0, width, height, linfo.format)!=GF_ERR_OK)
   {
      fprintf(stderr, "Unable to create rendering target\n");
      return -1;
   }

   gf_layer_set_src_viewport(layer, 0, 0, width-1, height-1);
   gf_layer_set_dst_viewport(layer, 0, 0, width-1, height-1);
   gf_layer_enable(layer);

   /*
    * The layer settings haven't taken effect yet since we haven't
    * called gf_layer_update() yet.  This is exactly what we want,
    * since we haven't supplied a valid surface to display yet.
    * Later, the OpenGL ES library calls will call gf_layer_update()
    * internally, when  displaying the rendered 3D content.
    */

   /* create an EGL rendering context */
   econtext=eglCreateContext(display, config, EGL_NO_CONTEXT, NULL);
   if (econtext==EGL_NO_CONTEXT)
   {
      fprintf(stderr, "Create context failed: 0x%x\n", eglGetError());
      return -1;
   }

   /* create an EGL window surface */
   surface=eglCreateWindowSurface(display, config, target, NULL);
   if (surface==EGL_NO_SURFACE)
   {
      fprintf(stderr, "Create surface failed: 0x%x\n", eglGetError());
      return -1;
   }

   /* connect the context to the surface */
   if (eglMakeCurrent(display, surface, surface, econtext)==EGL_FALSE)
   {
      fprintf(stderr, "Make current failed: 0x%x\n", eglGetError());
      return -1;
   }

   init_scene();

   do {
      render_scene();
      glFinish();
      eglWaitGL();
      eglSwapBuffers(display,surface);
   } while(1);

   return 0;
}
