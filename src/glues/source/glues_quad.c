/*
 * SGI FREE SOFTWARE LICENSE B (Version 2.0, Sept. 18, 2008)
 * Copyright (C) 1991-2000 Silicon Graphics, Inc. All Rights Reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice including the dates of first publication and
 * either this permission notice or a reference to
 * http://oss.sgi.com/projects/FreeB/
 * shall be included in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * SILICON GRAPHICS, INC. BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
 * WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
 * OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Except as contained in this notice, the name of Silicon Graphics, Inc.
 * shall not be used in advertising or otherwise to promote the sale, use or
 * other dealings in this Software without prior written authorization from
 * Silicon Graphics, Inc.
 *
 * OpenGL ES CM 1.0 port of part of GLU by Mike Gorchak <mike@malva.ua>.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "glues_quad.h"

/* Make it not a power of two to avoid cache thrashing on the chip */
#define CACHE_SIZE    240

#undef  PI
#define PI            3.14159265358979323846f

struct GLUquadric
{
    GLint       normals;
    GLboolean   textureCoords;
    GLint       orientation;
    GLint       drawStyle;
    void        (APIENTRY* errorCallback)( GLint );
};

GLAPI GLUquadric* APIENTRY gluNewQuadric(void)
{
   GLUquadric *newstate;

   newstate=(GLUquadric*)malloc(sizeof(GLUquadric));
   if (newstate==NULL)
   {
      /* Can't report an error at this point... */
      return NULL;
   }

   newstate->normals=GLU_SMOOTH;
   newstate->textureCoords=GL_FALSE;
   newstate->orientation=GLU_OUTSIDE;
   newstate->drawStyle=GLU_FILL;
   newstate->errorCallback=NULL;

   return newstate;
}

GLAPI void APIENTRY gluDeleteQuadric(GLUquadric* state)
{
   if (state!=NULL)
   {
      free(state);
   }
}

static void gluQuadricError(GLUquadric* qobj, GLenum which)
{
   if (qobj->errorCallback)
   {
      qobj->errorCallback(which);
   }
}

GLAPI void APIENTRY gluQuadricCallback(GLUquadric* qobj, GLenum which, _GLUfuncptr fn)
{
   switch (which)
   {
      case GLU_ERROR:
           qobj->errorCallback=(void(APIENTRY*)(GLint))fn;
           break;
      default:
           gluQuadricError(qobj, GLU_INVALID_ENUM);
           return;
   }
}

GLAPI void APIENTRY gluQuadricNormals(GLUquadric* qobj, GLenum normals)
{
   switch (normals)
   {
      case GLU_SMOOTH:
      case GLU_FLAT:
      case GLU_NONE:
           break;
      default:
           gluQuadricError(qobj, GLU_INVALID_ENUM);
           return;
   }
   qobj->normals=normals;
}

GLAPI void APIENTRY gluQuadricTexture(GLUquadric* qobj, GLboolean textureCoords)
{
   qobj->textureCoords=textureCoords;
}

GLAPI void APIENTRY gluQuadricOrientation(GLUquadric* qobj, GLenum orientation)
{
   switch(orientation)
   {
      case GLU_OUTSIDE:
      case GLU_INSIDE:
           break;
      default:
           gluQuadricError(qobj, GLU_INVALID_ENUM);
           return;
   }

   qobj->orientation=orientation;
}

GLAPI void APIENTRY gluQuadricDrawStyle(GLUquadric* qobj, GLenum drawStyle)
{
   switch(drawStyle)
   {
      case GLU_POINT:
      case GLU_LINE:
      case GLU_FILL:
      case GLU_SILHOUETTE:
           break;
      default:
           gluQuadricError(qobj, GLU_INVALID_ENUM);
           return;
   }

   qobj->drawStyle=drawStyle;
}

GLAPI void APIENTRY gluCylinder(GLUquadric* qobj, GLfloat baseRadius,
                                GLfloat topRadius, GLfloat height,
                                GLint slices, GLint stacks)
{
   GLint i, j;
   GLfloat sinCache[CACHE_SIZE];
   GLfloat cosCache[CACHE_SIZE];
   GLfloat sinCache2[CACHE_SIZE];
   GLfloat cosCache2[CACHE_SIZE];
   GLfloat sinCache3[CACHE_SIZE];
   GLfloat cosCache3[CACHE_SIZE];
   GLfloat sintemp, costemp;
   GLfloat vertices[(CACHE_SIZE+1)*2][3];
   GLfloat texcoords[(CACHE_SIZE+1)*2][2];
   GLfloat normals[(CACHE_SIZE+1)*2][3];
   GLfloat angle;
   GLfloat zLow, zHigh;
   GLfloat length;
   GLfloat deltaRadius;
   GLfloat zNormal;
   GLfloat xyNormalRatio;
   GLfloat radiusLow, radiusHigh;
   int needCache2, needCache3;
   GLboolean texcoord_enabled;
   GLboolean normal_enabled;
   GLboolean vertex_enabled;
   GLboolean color_enabled;

   if (slices>=CACHE_SIZE)
   {
      slices=CACHE_SIZE-1;
   }

   /* Avoid vertex array overflow */
   if (stacks>=CACHE_SIZE)
   {
      stacks=CACHE_SIZE-1;
   }

   if (slices<2 || stacks<1 || baseRadius<0.0 || topRadius<0.0 || height<0.0)
   {
      gluQuadricError(qobj, GLU_INVALID_VALUE);
      return;
   }

   /* Compute length (needed for normal calculations) */
   deltaRadius=baseRadius-topRadius;
   length=(GLfloat)sqrt(deltaRadius*deltaRadius+height*height);

   if (length==0.0)
   {
      gluQuadricError(qobj, GLU_INVALID_VALUE);
      return;
   }

   /* Cache is the vertex locations cache */
   /* Cache2 is the various normals at the vertices themselves */
   /* Cache3 is the various normals for the faces */
   needCache2=needCache3=0;
   if (qobj->normals==GLU_SMOOTH)
   {
      needCache2=1;
   }

   if (qobj->normals==GLU_FLAT)
   {
      if (qobj->drawStyle!=GLU_POINT)
      {
         needCache3=1;
      }
      if (qobj->drawStyle==GLU_LINE)
      {
         needCache2=1;
      }
   }

   zNormal=deltaRadius/length;
   xyNormalRatio=height/length;

   for (i=0; i<slices; i++)
   {
      angle=2.0f*PI*i/slices;
      if (needCache2)
      {
         if (qobj->orientation==GLU_OUTSIDE)
         {
            sinCache2[i]=(GLfloat)(xyNormalRatio*sin(angle));
            cosCache2[i]=(GLfloat)(xyNormalRatio*cos(angle));
         }
         else
         {
            sinCache2[i]=(GLfloat)(-xyNormalRatio*sin(angle));
            cosCache2[i]=(GLfloat)(-xyNormalRatio*cos(angle));
         }
      }

      sinCache[i]=(GLfloat)sin(angle);
      cosCache[i]=(GLfloat)cos(angle);
   }

   if (needCache3)
   {
      for (i=0; i<slices; i++)
      {
         angle=2.0f*PI*(i-0.5f)/slices;
         if (qobj->orientation==GLU_OUTSIDE)
         {
            sinCache3[i]=(GLfloat)(xyNormalRatio*sin(angle));
            cosCache3[i]=(GLfloat)(xyNormalRatio*cos(angle));
         }
         else
         {
            sinCache3[i]=(GLfloat)(-xyNormalRatio*sin(angle));
            cosCache3[i]=(GLfloat)(-xyNormalRatio*cos(angle));
         }
      }
   }

   sinCache[slices]=sinCache[0];
   cosCache[slices]=cosCache[0];
   if (needCache2)
   {
      sinCache2[slices]=sinCache2[0];
      cosCache2[slices]=cosCache2[0];
   }
   if (needCache3)
   {
      sinCache3[slices]=sinCache3[0];
      cosCache3[slices]=cosCache3[0];
   }

   /* Store status of enabled arrays */
   texcoord_enabled=GL_FALSE; //glIsEnabled(GL_TEXTURE_COORD_ARRAY);
   normal_enabled=GL_FALSE; //glIsEnabled(GL_NORMAL_ARRAY);
   vertex_enabled=GL_FALSE; //glIsEnabled(GL_VERTEX_ARRAY);
   color_enabled=GL_FALSE; //glIsEnabled(GL_COLOR_ARRAY);

   /* Enable or disable arrays */
   glEnableClientState(GL_VERTEX_ARRAY);
   glVertexPointer(3, GL_FLOAT, 0, vertices);
   if (qobj->textureCoords)
   {
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      glTexCoordPointer(2, GL_FLOAT, 0, texcoords);
   }
   else
   {
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
   }
   if (qobj->normals!=GLU_NONE)
   {
      glEnableClientState(GL_NORMAL_ARRAY);
      glNormalPointer(GL_FLOAT, 0, normals);
   }
   else
   {
      glDisableClientState(GL_NORMAL_ARRAY);
   }
   glDisableClientState(GL_COLOR_ARRAY);

   switch (qobj->drawStyle)
   {
      case GLU_FILL:
           for (j=0; j<stacks; j++)
           {
              zLow=j*height/stacks;
              zHigh=(j+1)*height/stacks;
              radiusLow=baseRadius-deltaRadius*((GLfloat)j/stacks);
              radiusHigh=baseRadius-deltaRadius*((GLfloat)(j+1)/stacks);

              for (i=0; i<=slices; i++)
              {
                 switch(qobj->normals)
                 {
                    case GLU_FLAT:
                         /* per vertex normals */
                         normals[i*2][0]=sinCache3[i];
                         normals[i*2][1]=cosCache3[i];
                         normals[i*2][2]=zNormal;
                         normals[i*2+1][0]=sinCache3[i];
                         normals[i*2+1][1]=cosCache3[i];
                         normals[i*2+1][2]=zNormal;
                         break;
                    case GLU_SMOOTH:
                         /* per vertex normals */
                         normals[i*2][0]=sinCache2[i];
                         normals[i*2][1]=cosCache2[i];
                         normals[i*2][2]=zNormal;
                         normals[i*2+1][0]=sinCache2[i];
                         normals[i*2+1][1]=cosCache2[i];
                         normals[i*2+1][2]=zNormal;
                         break;
                    case GLU_NONE:
                    default:
                         break;
                 }
                 if (qobj->orientation==GLU_OUTSIDE)
                 {
                    if (qobj->textureCoords)
                    {
                       texcoords[i*2][0]=1-(GLfloat)i/slices;
                       texcoords[i*2][1]=(GLfloat)j/stacks;
                    }
                    vertices[i*2][0]=radiusLow*sinCache[i];
                    vertices[i*2][1]=radiusLow*cosCache[i];
                    vertices[i*2][2]=zLow;

                    if (qobj->textureCoords)
                    {
                       texcoords[i*2+1][0]=1-(GLfloat)i/slices;
                       texcoords[i*2+1][1]=(GLfloat)(j+1)/stacks;
                    }
                    vertices[i*2+1][0]=radiusHigh*sinCache[i];
                    vertices[i*2+1][1]=radiusHigh*cosCache[i];
                    vertices[i*2+1][2]=zHigh;
                 }
                 else
                 {
                    if (qobj->textureCoords)
                    {
                       texcoords[i*2][0]=1-(GLfloat)i/slices;
                       texcoords[i*2][1]=(GLfloat)(j+1)/ stacks;
                    }
                    vertices[i*2][0]=radiusHigh*sinCache[i];
                    vertices[i*2][1]=radiusHigh*cosCache[i];
                    vertices[i*2][2]=zHigh;
                    if (qobj->textureCoords)
                    {
                       texcoords[i*2+1][0]=1-(GLfloat)i/slices;
                       texcoords[i*2+1][1]=(GLfloat)j/stacks;
                    }
                    vertices[i*2+1][0]=radiusLow*sinCache[i];
                    vertices[i*2+1][1]=radiusLow*cosCache[i];
                    vertices[i*2+1][2]=zLow;
                 }
              }
              glDrawArrays(GL_TRIANGLE_STRIP, 0, 2*(slices+1));
           }
           break;
      case GLU_POINT:
           for (i=0; i<slices; i++)
           {
              sintemp=sinCache[i];
              costemp=cosCache[i];

              for (j=0; j<=stacks; j++)
              {
                 switch(qobj->normals)
                 {
                    case GLU_FLAT:
                    case GLU_SMOOTH:
                         normals[j][0]=sinCache2[i];
                         normals[j][1]=cosCache2[i];
                         normals[j][2]=zNormal;
                         break;
                    case GLU_NONE:
                    default:
                         break;
                 }

                 zLow=j*height/stacks;
                 radiusLow=baseRadius-deltaRadius*((GLfloat)j/stacks);

                 if (qobj->textureCoords)
                 {
                    texcoords[j][0]=1-(GLfloat)i/slices;
                    texcoords[j][1]=(GLfloat)j/stacks;
                 }
                 vertices[j][0]=radiusLow*sintemp;
                 vertices[j][1]=radiusLow*costemp;
                 vertices[j][2]=zLow;
              }
              glDrawArrays(GL_POINTS, 0, (stacks+1));
           }
           break;
      case GLU_LINE:
           for (j=1; j<stacks; j++)
           {
              zLow=j*height/stacks;
              radiusLow=baseRadius-deltaRadius*((GLfloat)j/stacks);

              for (i=0; i<=slices; i++)
              {
                 switch(qobj->normals)
                 {
                    case GLU_FLAT:
                         normals[i][0]=sinCache3[i];
                         normals[i][1]=cosCache3[i];
                         normals[i][2]=zNormal;
                         break;
                    case GLU_SMOOTH:
                         normals[i][0]=sinCache2[i];
                         normals[i][1]=cosCache2[i];
                         normals[i][2]=zNormal;
                         break;
                    case GLU_NONE:
                    default:
                         break;
                 }
                 if (qobj->textureCoords)
                 {
                    texcoords[i][0]=1-(GLfloat)i/slices;
                    texcoords[i][1]=(GLfloat)j/stacks;
                 }
                 vertices[i][0]=radiusLow*sinCache[i];
                 vertices[i][1]=radiusLow*cosCache[i];
                 vertices[i][2]=zLow;
              }
              glDrawArrays(GL_LINE_STRIP, 0, (slices+1));
           }
           /* Intentionally fall through here... */
      case GLU_SILHOUETTE:
           for (j=0; j<=stacks; j+=stacks)
           {
              zLow=j*height/stacks;
              radiusLow=baseRadius-deltaRadius*((GLfloat)j/stacks);

              for (i=0; i<=slices; i++)
              {
                 switch(qobj->normals)
                 {
                    case GLU_FLAT:
                         normals[i][0]=sinCache3[i];
                         normals[i][1]=cosCache3[i];
                         normals[i][2]=zNormal;
                         break;
                    case GLU_SMOOTH:
                         normals[i][0]=sinCache2[i];
                         normals[i][1]=cosCache2[i];
                         normals[i][2]=zNormal;
                         break;
                    case GLU_NONE:
                    default:
                         break;
                 }
                 if (qobj->textureCoords)
                 {
                    texcoords[i][0]=1-(GLfloat)i/slices;
                    texcoords[i][1]=(GLfloat)j/stacks;
                 }
                 vertices[i][0]=radiusLow*sinCache[i];
                 vertices[i][1]=radiusLow*cosCache[i];
                 vertices[i][2]=zLow;
              }
              glDrawArrays(GL_LINE_STRIP, 0, (slices+1));
           }

           for (i=0; i<slices; i++)
           {
              sintemp=sinCache[i];
              costemp=cosCache[i];

              for (j=0; j<=stacks; j++)
              {
                 switch(qobj->normals)
                 {
                    case GLU_FLAT:
                    case GLU_SMOOTH:
                         normals[j][0]=sinCache2[i];
                         normals[j][1]=cosCache2[i];
                         normals[j][2]=0.0f;
                         break;
                    case GLU_NONE:
                    default:
                         break;
                 }

                 zLow=j*height/stacks;
                 radiusLow=baseRadius-deltaRadius*((GLfloat)j/stacks);

                 if (qobj->textureCoords)
                 {
                    texcoords[j][0]=1-(GLfloat)i/slices;
                    texcoords[j][1]=(GLfloat)j/stacks;
                 }
                 vertices[j][0]=radiusLow*sintemp;
                 vertices[j][1]=radiusLow*costemp;
                 vertices[j][2]=zLow;
              }
              glDrawArrays(GL_LINE_STRIP, 0, (stacks+1));
           }
           break;
      default:
           break;
   }

   /* Disable or re-enable arrays */
   if (vertex_enabled)
   {
      /* Re-enable vertex array */
      glEnableClientState(GL_VERTEX_ARRAY);
   }
   else
   {
      glDisableClientState(GL_VERTEX_ARRAY);
   }

   if (texcoord_enabled)
   {
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
   }
   else
   {
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
   }

   if (normal_enabled)
   {
      glEnableClientState(GL_NORMAL_ARRAY);
   }
   else
   {
      glDisableClientState(GL_NORMAL_ARRAY);
   }

   if (color_enabled)
   {
      glEnableClientState(GL_COLOR_ARRAY);
   }
   else
   {
      glDisableClientState(GL_COLOR_ARRAY);
   }
}

GLAPI void APIENTRY gluDisk(GLUquadric* qobj, GLfloat innerRadius,
                            GLfloat outerRadius, GLint slices, GLint loops)
{
   gluPartialDisk(qobj, innerRadius, outerRadius, slices, loops, 0.0, 360.0);
}

GLAPI void APIENTRY gluPartialDisk(GLUquadric* qobj, GLfloat innerRadius,
                                   GLfloat outerRadius, GLint slices,
                                   GLint loops, GLfloat startAngle,
                                   GLfloat sweepAngle)
{
   GLint i, j;
   GLfloat sinCache[CACHE_SIZE];
   GLfloat cosCache[CACHE_SIZE];
   GLfloat angle;
   GLfloat sintemp, costemp;
   GLfloat vertices[(CACHE_SIZE+1)*2][3];
   GLfloat texcoords[(CACHE_SIZE+1)*2][2];
   GLfloat deltaRadius;
   GLfloat radiusLow, radiusHigh;
   GLfloat texLow = 0.0, texHigh = 0.0;
   GLfloat angleOffset;
   GLint slices2;
   GLint finish;
   GLboolean texcoord_enabled;
   GLboolean normal_enabled;
   GLboolean vertex_enabled;
   GLboolean color_enabled;

   if (slices>=CACHE_SIZE)
   {
      slices=CACHE_SIZE-1;
   }

   if (slices<2 || loops<1 || outerRadius<=0.0 ||
       innerRadius<0.0 ||innerRadius > outerRadius)
   {
      gluQuadricError(qobj, GLU_INVALID_VALUE);
      return;
   }

   if (sweepAngle<-360.0)
   {
      sweepAngle=-360.0;
   }
   if (sweepAngle>360.0)
   {
      sweepAngle=360.0;
   }

   if (sweepAngle<0)
   {
      startAngle+=sweepAngle;
      sweepAngle=-sweepAngle;
   }

   if (sweepAngle==360.0)
   {
      slices2=slices;
   }
   else
   {
      slices2=slices+1;
   }

   /* Compute length (needed for normal calculations) */
   deltaRadius=outerRadius-innerRadius;

   /* Cache is the vertex locations cache */
   angleOffset=startAngle/180.0f*PI;
   for (i=0; i<=slices; i++)
   {
      angle=angleOffset+((PI*sweepAngle)/180.0f)*i/slices;
      sinCache[i]=(GLfloat)sin(angle);
      cosCache[i]=(GLfloat)cos(angle);
   }

   if (sweepAngle==360.0)
   {
      sinCache[slices]=sinCache[0];
      cosCache[slices]=cosCache[0];
   }

   switch(qobj->normals)
   {
      case GLU_FLAT:
      case GLU_SMOOTH:
           if (qobj->orientation==GLU_OUTSIDE)
           {
              glNormal3f(0.0f, 0.0f, 1.0f);
           }
           else
           {
              glNormal3f(0.0f, 0.0f, -1.0f);
           }
           break;
      default:
      case GLU_NONE:
           break;
   }

   /* Store status of enabled arrays */
   texcoord_enabled=GL_FALSE; //glIsEnabled(GL_TEXTURE_COORD_ARRAY);
   normal_enabled=GL_FALSE; //glIsEnabled(GL_NORMAL_ARRAY);
   vertex_enabled=GL_FALSE; //glIsEnabled(GL_VERTEX_ARRAY);
   color_enabled=GL_FALSE; //glIsEnabled(GL_COLOR_ARRAY);

   /* Enable arrays */
   glEnableClientState(GL_VERTEX_ARRAY);
   glVertexPointer(3, GL_FLOAT, 0, vertices);
   if (qobj->textureCoords)
   {
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      glTexCoordPointer(2, GL_FLOAT, 0, texcoords);
   }
   else
   {
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
   }
   glDisableClientState(GL_NORMAL_ARRAY);
   glDisableClientState(GL_COLOR_ARRAY);

   switch (qobj->drawStyle)
   {
      case GLU_FILL:
           if (innerRadius==0.0)
           {
              finish=loops-1;

              /* Triangle strip for inner polygons */
              if (qobj->textureCoords)
              {
                 texcoords[0][0]=0.5f;
                 texcoords[0][1]=0.5f;
              }
              vertices[0][0]=0.0f;
              vertices[0][1]=0.0f;
              vertices[0][2]=0.0f;
              radiusLow=outerRadius-deltaRadius*((GLfloat)(loops-1)/loops);
              if (qobj->textureCoords)
              {
                 texLow=radiusLow/outerRadius/2;
              }

              if (qobj->orientation==GLU_OUTSIDE)
              {
                 for (i=slices; i>=0; i--)
                 {
                    if (qobj->textureCoords)
                    {
                       texcoords[slices-i+1][0]=texLow*sinCache[i]+0.5f;
                       texcoords[slices-i+1][1]=texLow*cosCache[i]+0.5f;
                    }
                    vertices[slices-i+1][0]=radiusLow*sinCache[i];
                    vertices[slices-i+1][1]=radiusLow*cosCache[i];
                    vertices[slices-i+1][2]=0.0f;
                 }
              }
              else
              {
                 for (i=0; i<=slices; i++)
                 {
                    if (qobj->textureCoords)
                    {
                       texcoords[i+1][0]=texLow*sinCache[i]+0.5f;
                       texcoords[i+1][1]=texLow*cosCache[i]+0.5f;
                    }
                    vertices[i+1][0]=radiusLow*sinCache[i];
                    vertices[i+1][1]=radiusLow*cosCache[i];
                    vertices[i+1][2]=0.0f;
                 }
              }
              glDrawArrays(GL_TRIANGLE_FAN, 0, (slices+2));
           }
           else
           {
              finish=loops;
           }

           for (j=0; j<finish; j++)
           {
              radiusLow=outerRadius-deltaRadius*((GLfloat)j/loops);
              radiusHigh=outerRadius-deltaRadius*((GLfloat)(j+1)/loops);
              if (qobj->textureCoords)
              {
                 texLow=radiusLow/outerRadius/2;
                 texHigh=radiusHigh/outerRadius/2;
              }

              for (i=0; i<=slices; i++)
              {
                 if (qobj->orientation==GLU_OUTSIDE)
                 {
                    if (qobj->textureCoords)
                    {
                       texcoords[i*2][0]=texLow*sinCache[i]+0.5f;
                       texcoords[i*2][1]=texLow*cosCache[i]+0.5f;
                    }
                    vertices[i*2][0]=radiusLow*sinCache[i];
                    vertices[i*2][1]=radiusLow*cosCache[i];
                    vertices[i*2][2]=0.0;

                    if (qobj->textureCoords)
                    {
                       texcoords[i*2+1][0]=texHigh*sinCache[i]+0.5f;
                       texcoords[i*2+1][1]=texHigh*cosCache[i]+0.5f;
                    }
                    vertices[i*2+1][0]=radiusHigh*sinCache[i];
                    vertices[i*2+1][1]=radiusHigh*cosCache[i];
                    vertices[i*2+1][2]=0.0;
                 }
                 else
                 {
                    if (qobj->textureCoords)
                    {
                       texcoords[i*2][0]=texHigh*sinCache[i]+0.5f;
                       texcoords[i*2][1]=texHigh*cosCache[i]+0.5f;
                    }
                    vertices[i*2][0]=radiusHigh*sinCache[i];
                    vertices[i*2][1]=radiusHigh*cosCache[i];
                    vertices[i*2][2]=0.0;

                    if (qobj->textureCoords)
                    {
                       texcoords[i*2+1][0]=texLow*sinCache[i]+0.5f;
                       texcoords[i*2+1][1]=texLow*cosCache[i]+0.5f;
                    }
                    vertices[i*2+1][0]=radiusLow*sinCache[i];
                    vertices[i*2+1][1]=radiusLow*cosCache[i];
                    vertices[i*2+1][2]=0.0;
                 }
              }
              glDrawArrays(GL_TRIANGLE_STRIP, 0, ((slices+1)*2));
           }
           break;
      case GLU_POINT:
           for (j=0; j<=loops; j++)
           {
              radiusLow=outerRadius-deltaRadius*((GLfloat)j/loops);

              for (i=0; i<slices2; i++)
              {
                 sintemp=sinCache[i];
                 costemp=cosCache[i];

                 if (qobj->textureCoords)
                 {
                    texLow=radiusLow/outerRadius/2;
                    texcoords[i][0]=texLow*sinCache[i]+0.5f;
                    texcoords[i][1]=texLow*cosCache[i]+0.5f;
                 }
                 vertices[i][0]=radiusLow*sintemp;
                 vertices[i][1]=radiusLow*costemp;
                 vertices[i][2]=0.0f;
              }
              glDrawArrays(GL_POINTS, 0, slices2);
           }
           break;
      case GLU_LINE:
           if (innerRadius==outerRadius)
           {
              for (i=0; i<=slices; i++)
              {
                 if (qobj->textureCoords)
                 {
                    texcoords[i][0]=sinCache[i]/2+0.5f;
                    texcoords[i][1]=cosCache[i]/2+0.5f;
                 }
                 vertices[i][0]=innerRadius*sinCache[i];
                 vertices[i][1]=innerRadius*cosCache[i];
                 vertices[i][2]=0.0f;
              }
              glDrawArrays(GL_LINE_STRIP, 0, slices+1);
              break;
           }

           for (j=0; j<=loops; j++)
           {
              radiusLow=outerRadius-deltaRadius*((GLfloat)j/loops);
              if (qobj->textureCoords)
              {
                 texLow=radiusLow/outerRadius/2;
              }

              for (i=0; i<=slices; i++)
              {
                 if (qobj->textureCoords)
                 {
                    texcoords[i][0]=texLow*sinCache[i]+0.5f;
                    texcoords[i][1]=texLow*cosCache[i]+0.5f;
                 }
                 vertices[i][0]=radiusLow*sinCache[i];
                 vertices[i][1]=radiusLow*cosCache[i];
                 vertices[i][2]=0.0f;
              }
              glDrawArrays(GL_LINE_STRIP, 0, slices+1);
           }

           for (i=0; i<slices2; i++)
           {
              sintemp=sinCache[i];
              costemp=cosCache[i];
              for (j=0; j<=loops; j++)
              {
                 radiusLow=outerRadius-deltaRadius*((GLfloat)j/loops);
                 if (qobj->textureCoords)
                 {
                    texLow=radiusLow/outerRadius/2;
                 }

                 if (qobj->textureCoords)
                 {
                    texcoords[j][0]=texLow*sinCache[i]+0.5f;
                    texcoords[j][1]=texLow*cosCache[i]+0.5f;
                 }
                 vertices[j][0]=radiusLow*sintemp;
                 vertices[j][1]=radiusLow*costemp;
                 vertices[j][2]=0.0f;
              }
              glDrawArrays(GL_LINE_STRIP, 0, loops+1);
           }
           break;
      case GLU_SILHOUETTE:
           if (sweepAngle<360.0)
           {
              for (i=0; i<=slices; i+=slices)
              {
                 sintemp=sinCache[i];
                 costemp=cosCache[i];

                 for (j=0; j<=loops; j++)
                 {
                    radiusLow=outerRadius-deltaRadius*((GLfloat)j/loops);

                    if (qobj->textureCoords)
                    {
                       texLow=radiusLow/outerRadius/2;
                       texcoords[j][0]=texLow*sinCache[i]+0.5f;
                       texcoords[j][1]=texLow*cosCache[i]+0.5f;
                    }
                    vertices[j][0]=radiusLow*sintemp;
                    vertices[j][1]=radiusLow*costemp;
                    vertices[j][2]=0.0f;
                 }
                 glDrawArrays(GL_LINE_STRIP, 0, loops+1);
              }
           }

           for (j=0; j<=loops; j+=loops)
           {
              radiusLow=outerRadius-deltaRadius*((GLfloat)j/loops);
              if (qobj->textureCoords)
              {
                 texLow=radiusLow/outerRadius/2;
              }

              for (i=0; i<=slices; i++)
              {
                 if (qobj->textureCoords)
                 {
                    texcoords[i][0]=texLow*sinCache[i]+0.5f;
                    texcoords[i][1]=texLow*cosCache[i]+0.5f;
                 }
                 vertices[i][0]=radiusLow*sinCache[i];
                 vertices[i][1]=radiusLow*cosCache[i];
                 vertices[i][2]=0.0f;
              }
              glDrawArrays(GL_LINE_STRIP, 0, slices+1);

              if (innerRadius==outerRadius)
              {
                 break;
              }
           }
           break;
      default:
           break;
   }

   /* Disable or re-enable arrays */
   if (vertex_enabled)
   {
      /* Re-enable vertex array */
      glEnableClientState(GL_VERTEX_ARRAY);
   }
   else
   {
      glDisableClientState(GL_VERTEX_ARRAY);
   }

   if (texcoord_enabled)
   {
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
   }
   else
   {
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
   }

   if (normal_enabled)
   {
      glEnableClientState(GL_NORMAL_ARRAY);
   }
   else
   {
      glDisableClientState(GL_NORMAL_ARRAY);
   }

   if (color_enabled)
   {
      glEnableClientState(GL_COLOR_ARRAY);
   }
   else
   {
      glDisableClientState(GL_COLOR_ARRAY);
   }
}

GLAPI void APIENTRY gluSphere(GLUquadric* qobj, GLfloat radius, GLint slices, GLint stacks)
{
   GLint i, j;
   GLfloat sinCache1a[CACHE_SIZE];
   GLfloat cosCache1a[CACHE_SIZE];
   GLfloat sinCache2a[CACHE_SIZE];
   GLfloat cosCache2a[CACHE_SIZE];
   GLfloat sinCache3a[CACHE_SIZE];
   GLfloat cosCache3a[CACHE_SIZE];
   GLfloat sinCache1b[CACHE_SIZE];
   GLfloat cosCache1b[CACHE_SIZE];
   GLfloat sinCache2b[CACHE_SIZE];
   GLfloat cosCache2b[CACHE_SIZE];
   GLfloat sinCache3b[CACHE_SIZE];
   GLfloat cosCache3b[CACHE_SIZE];
   GLfloat angle;
   GLfloat zLow, zHigh;
   GLfloat sintemp1 = 0.0, sintemp2 = 0.0, sintemp3 = 0.0, sintemp4 = 0.0;
   GLfloat costemp1 = 0.0, costemp2 = 0.0, costemp3 = 0.0, costemp4 = 0.0;
   GLfloat vertices[(CACHE_SIZE+1)*2][3];
   GLfloat texcoords[(CACHE_SIZE+1)*2][2];
   GLfloat normals[(CACHE_SIZE+1)*2][3];
   GLboolean needCache2, needCache3;
   GLint start, finish;
   GLboolean texcoord_enabled;
   GLboolean normal_enabled;
   GLboolean vertex_enabled;
   GLboolean color_enabled;

   if (slices>=CACHE_SIZE)
   {
      slices=CACHE_SIZE-1;
   }

   if (stacks>=CACHE_SIZE)
   {
      stacks=CACHE_SIZE-1;
   }

   if (slices<2 || stacks<1 || radius<0.0)
   {
      gluQuadricError(qobj, GLU_INVALID_VALUE);
      return;
   }

   /* Cache is the vertex locations cache */
   /* Cache2 is the various normals at the vertices themselves */
   /* Cache3 is the various normals for the faces */
   needCache2=needCache3=GL_FALSE;

   if (qobj->normals==GLU_SMOOTH)
   {
      needCache2=GL_TRUE;
   }

   if (qobj->normals==GLU_FLAT)
   {
      if (qobj->drawStyle!=GLU_POINT)
      {
         needCache3=GL_TRUE;
      }
      if (qobj->drawStyle==GLU_LINE)
      {
         needCache2=GL_TRUE;
      }
   }

   for (i=0; i<slices; i++)
   {
      angle=2.0f*PI*i/slices;
      sinCache1a[i]=(GLfloat)sin(angle);
      cosCache1a[i]=(GLfloat)cos(angle);
      if (needCache2)
      {
         sinCache2a[i] = sinCache1a[i];
         cosCache2a[i] = cosCache1a[i];
      }
   }

   for (j=0; j<=stacks; j++)
   {
      angle=PI*j/stacks;
      if (needCache2)
      {
         if (qobj->orientation==GLU_OUTSIDE)
         {
            sinCache2b[j]=(GLfloat)sin(angle);
            cosCache2b[j]=(GLfloat)cos(angle);
         }
         else
         {
            sinCache2b[j]=(GLfloat)-sin(angle);
            cosCache2b[j]=(GLfloat)-cos(angle);
         }
      }

      sinCache1b[j]=(GLfloat)(radius*sin(angle));
      cosCache1b[j]=(GLfloat)(radius*cos(angle));
   }

   /* Make sure it comes to a point */
   sinCache1b[0]=0;
   sinCache1b[stacks]=0;

   if (needCache3)
   {
      for (i=0; i<slices; i++)
      {
         angle=2.0f*PI*(i-0.5f)/slices;
         sinCache3a[i]=(GLfloat)sin(angle);
         cosCache3a[i]=(GLfloat)cos(angle);
      }
      for (j=0; j<=stacks; j++)
      {
         angle=PI*(j-0.5f)/stacks;
         if (qobj->orientation==GLU_OUTSIDE)
         {
            sinCache3b[j]=(GLfloat)sin(angle);
            cosCache3b[j]=(GLfloat)cos(angle);
         }
         else
         {
            sinCache3b[j]=(GLfloat)-sin(angle);
            cosCache3b[j]=(GLfloat)-cos(angle);
         }
      }
   }

   sinCache1a[slices]=sinCache1a[0];
   cosCache1a[slices]=cosCache1a[0];
   if (needCache2)
   {
      sinCache2a[slices]=sinCache2a[0];
      cosCache2a[slices]=cosCache2a[0];
   }
   if (needCache3)
   {
      sinCache3a[slices]=sinCache3a[0];
      cosCache3a[slices]=cosCache3a[0];
   }

   /* Store status of enabled arrays */
   texcoord_enabled=GL_FALSE; //glIsEnabled(GL_TEXTURE_COORD_ARRAY);
   normal_enabled=GL_FALSE; //glIsEnabled(GL_NORMAL_ARRAY);
   vertex_enabled=GL_FALSE; //glIsEnabled(GL_VERTEX_ARRAY);
   color_enabled=GL_FALSE; //glIsEnabled(GL_COLOR_ARRAY);

   /* Enable arrays */
   glEnableClientState(GL_VERTEX_ARRAY);
   glVertexPointer(3, GL_FLOAT, 0, vertices);
   if (qobj->textureCoords)
   {
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
      glTexCoordPointer(2, GL_FLOAT, 0, texcoords);
   }
   else
   {
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
   }
   if (qobj->normals!=GLU_NONE)
   {
      glEnableClientState(GL_NORMAL_ARRAY);
      glNormalPointer(GL_FLOAT, 0, normals);
   }
   else
   {
      glDisableClientState(GL_NORMAL_ARRAY);
   }
   glDisableClientState(GL_COLOR_ARRAY);

   switch (qobj->drawStyle)
   {
      case GLU_FILL:
           if (!(qobj->textureCoords))
           {
              start=1;
              finish=stacks-1;

              /* Low end first (j == 0 iteration) */
              sintemp2=sinCache1b[1];
              zHigh=cosCache1b[1];

              switch(qobj->normals)
              {
                 case GLU_FLAT:
                      sintemp3=sinCache3b[1];
                      costemp3=cosCache3b[1];
                      normals[0][0]=sinCache3a[0]*sinCache3b[0];
                      normals[0][1]=cosCache3a[0]*sinCache3b[0];
                      normals[0][2]=cosCache3b[0];
                      break;
                 case GLU_SMOOTH:
                      sintemp3=sinCache2b[1];
                      costemp3=cosCache2b[1];
                      normals[0][0]=sinCache2a[0]*sinCache2b[0];
                      normals[0][1]=cosCache2a[0]*sinCache2b[0];
                      normals[0][2]=cosCache2b[0];
                      break;
                 default:
                      break;
              }

              vertices[0][0]=0.0f;
              vertices[0][1]=0.0f;
              vertices[0][2]=radius;

              if (qobj->orientation==GLU_OUTSIDE)
              {
                 for (i=slices; i>=0; i--)
                 {
                    switch(qobj->normals)
                    {
                       case GLU_SMOOTH:
                            normals[slices-i+1][0]=sinCache2a[i]*sintemp3;
                            normals[slices-i+1][1]=cosCache2a[i]*sintemp3;
                            normals[slices-i+1][2]=costemp3;
                            break;
                       case GLU_FLAT:
                            if (i!=slices)
                            {
                               normals[slices-i+1][0]=sinCache3a[i+1]*sintemp3;
                               normals[slices-i+1][1]=cosCache3a[i+1]*sintemp3;
                               normals[slices-i+1][2]=costemp3;
                            }
                            else
                            {
                               /* We must add any normal here */
                               normals[slices-i+1][0]=sinCache3a[i]*sintemp3;
                               normals[slices-i+1][1]=cosCache3a[i]*sintemp3;
                               normals[slices-i+1][2]=costemp3;
                            }
                            break;
                       case GLU_NONE:
                       default:
                            break;
                    }
                    vertices[slices-i+1][0]=sintemp2*sinCache1a[i];
                    vertices[slices-i+1][1]=sintemp2*cosCache1a[i];
                    vertices[slices-i+1][2]=zHigh;
                 }
              }
              else
              {
                 for (i=0; i<=slices; i++)
                 {
                    switch(qobj->normals)
                    {
                       case GLU_SMOOTH:
                            normals[i+1][0]=sinCache2a[i]*sintemp3;
                            normals[i+1][1]=cosCache2a[i]*sintemp3;
                            normals[i+1][2]=costemp3;
                            break;
                       case GLU_FLAT:
                            normals[i+1][0]=sinCache3a[i]*sintemp3;
                            normals[i+1][1]=cosCache3a[i]*sintemp3;
                            normals[i+1][2]=costemp3;
                            break;
                       case GLU_NONE:
                       default:
                            break;
                    }
                    vertices[i+1][0]=sintemp2*sinCache1a[i];
                    vertices[i+1][1]=sintemp2*cosCache1a[i];
                    vertices[i+1][2]=zHigh;
                 }
              }
              glDrawArrays(GL_TRIANGLE_FAN, 0, (slices+2));

              /* High end next (j==stacks-1 iteration) */
              sintemp2=sinCache1b[stacks-1];
              zHigh=cosCache1b[stacks-1];
              switch(qobj->normals)
              {
                 case GLU_FLAT:
                      sintemp3=sinCache3b[stacks];
                      costemp3=cosCache3b[stacks];
                      normals[0][0]=sinCache3a[stacks]*sinCache3b[stacks];
                      normals[0][1]=cosCache3a[stacks]*sinCache3b[stacks];
                      normals[0][2]=cosCache3b[stacks];
                      break;
                 case GLU_SMOOTH:
                      sintemp3=sinCache2b[stacks-1];
                      costemp3=cosCache2b[stacks-1];
                      normals[0][0]=sinCache2a[stacks]*sinCache2b[stacks];
                      normals[0][1]=cosCache2a[stacks]*sinCache2b[stacks];
                      normals[0][2]=cosCache2b[stacks];
                      break;
                 default:
                      break;
              }

              vertices[0][0]=0.0f;
              vertices[0][1]=0.0f;
              vertices[0][2]=-radius;

              if (qobj->orientation==GLU_OUTSIDE)
              {
                 for (i=0; i<=slices; i++)
                 {
                    switch(qobj->normals)
                    {
                       case GLU_SMOOTH:
                            normals[i+1][0]=sinCache2a[i]*sintemp3;
                            normals[i+1][1]=cosCache2a[i]*sintemp3;
                            normals[i+1][2]=costemp3;
                            break;
                       case GLU_FLAT:
                            normals[i+1][0]=sinCache3a[i]*sintemp3;
                            normals[i+1][1]=cosCache3a[i]*sintemp3;
                            normals[i+1][2]=costemp3;
                            break;
                       case GLU_NONE:
                       default:
                            break;
                    }
                    vertices[i+1][0]=sintemp2*sinCache1a[i];
                    vertices[i+1][1]=sintemp2*cosCache1a[i];
                    vertices[i+1][2]=zHigh;
                 }
              }
              else
              {
                 for (i=slices; i>=0; i--)
                 {
                    switch(qobj->normals)
                    {
                       case GLU_SMOOTH:
                            normals[slices-i+1][0]=sinCache2a[i]*sintemp3;
                            normals[slices-i+1][1]=cosCache2a[i]*sintemp3;
                            normals[slices-i+1][2]=costemp3;
                            break;
                       case GLU_FLAT:
                            if (i!=slices)
                            {
                               normals[slices-i+1][0]=sinCache3a[i+1]*sintemp3;
                               normals[slices-i+1][1]=cosCache3a[i+1]*sintemp3;
                               normals[slices-i+1][2]=costemp3;
                            }
                            else
                            {
                               normals[slices-i+1][0]=sinCache3a[i]*sintemp3;
                               normals[slices-i+1][1]=cosCache3a[i]*sintemp3;
                               normals[slices-i+1][2]=costemp3;
                            }
                            break;
                       case GLU_NONE:
                       default:
                            break;
                    }
                    vertices[slices-i+1][0]=sintemp2*sinCache1a[i];
                    vertices[slices-i+1][1]=sintemp2*cosCache1a[i];
                    vertices[slices-i+1][2]=zHigh;
                 }
              }
              glDrawArrays(GL_TRIANGLE_FAN, 0, (slices+2));
           }
           else
           {
              start=0;
              finish=stacks;
           }

           for (j=start; j<finish; j++)
           {
              zLow=cosCache1b[j];
              zHigh=cosCache1b[j+1];
              sintemp1=sinCache1b[j];
              sintemp2=sinCache1b[j+1];
              switch(qobj->normals)
              {
                 case GLU_FLAT:
                      sintemp4=sinCache3b[j+1];
                      costemp4=cosCache3b[j+1];
                      break;
                 case GLU_SMOOTH:
                      if (qobj->orientation==GLU_OUTSIDE)
                      {
                         sintemp3=sinCache2b[j+1];
                         costemp3=cosCache2b[j+1];
                         sintemp4=sinCache2b[j];
                         costemp4=cosCache2b[j];
                      }
                      else
                      {
                         sintemp3=sinCache2b[j];
                         costemp3=cosCache2b[j];
                         sintemp4=sinCache2b[j+1];
                         costemp4=cosCache2b[j+1];
                      }
                      break;
                 default:
                      break;
              }
              for (i=0; i<=slices; i++)
              {
                 switch(qobj->normals)
                 {
                    case GLU_SMOOTH:
                         normals[i*2][0]=sinCache2a[i]*sintemp3;
                         normals[i*2][1]=cosCache2a[i]*sintemp3;
                         normals[i*2][2]=costemp3;
                         break;
                    case GLU_FLAT:
                         normals[i*2][0]=sinCache3a[i]*sintemp4;
                         normals[i*2][1]=cosCache3a[i]*sintemp4;
                         normals[i*2][2]=costemp4;
                         break;
                    case GLU_NONE:
                    default:
                         break;
                 }
                 if (qobj->orientation==GLU_OUTSIDE)
                 {
                    if (qobj->textureCoords)
                    {
                       texcoords[i*2][0]=1-(GLfloat)i/slices;
                       texcoords[i*2][1]=1-(GLfloat)(j+1)/stacks;
                    }
                    vertices[i*2][0]=sintemp2*sinCache1a[i];
                    vertices[i*2][1]=sintemp2*cosCache1a[i];
                    vertices[i*2][2]=zHigh;
                 }
                 else
                 {
                    if (qobj->textureCoords)
                    {
                       texcoords[i*2][0]=1-(GLfloat)i/slices;
                       texcoords[i*2][1]=1-(GLfloat)j/stacks;
                    }
                    vertices[i*2][0]=sintemp1*sinCache1a[i];
                    vertices[i*2][1]=sintemp1*cosCache1a[i];
                    vertices[i*2][2]=zLow;
                 }
                 switch(qobj->normals)
                 {
                    case GLU_SMOOTH:
                         normals[i*2+1][0]=sinCache2a[i]*sintemp4;
                         normals[i*2+1][1]=cosCache2a[i]*sintemp4;
                         normals[i*2+1][2]=costemp4;
                         break;
                    case GLU_FLAT:
                         normals[i*2+1][0]=sinCache3a[i]*sintemp4;
                         normals[i*2+1][1]=cosCache3a[i]*sintemp4;
                         normals[i*2+1][2]=costemp4;
                         break;
                    case GLU_NONE:
                    default:
                         break;
                 }
                 if (qobj->orientation==GLU_OUTSIDE)
                 {
                    if (qobj->textureCoords)
                    {
                       texcoords[i*2+1][0]=1-(GLfloat)i/slices;
                       texcoords[i*2+1][1]=1-(GLfloat)j/stacks;
                    }
                    vertices[i*2+1][0]=sintemp1*sinCache1a[i];
                    vertices[i*2+1][1]=sintemp1*cosCache1a[i];
                    vertices[i*2+1][2]=zLow;
                 }
                 else
                 {
                    if (qobj->textureCoords)
                    {
                       texcoords[i*2+1][0]=1-(GLfloat)i/slices;
                       texcoords[i*2+1][1]=1-(GLfloat)(j+1)/stacks;
                    }
                    vertices[i*2+1][0]=sintemp2*sinCache1a[i];
                    vertices[i*2+1][1]=sintemp2*cosCache1a[i];
                    vertices[i*2+1][2]=zHigh;
                 }
              }
              glDrawArrays(GL_TRIANGLE_STRIP, 0, (slices+1)*2);
           }
           break;
      case GLU_POINT:
           for (j=0; j<=stacks; j++)
           {
              sintemp1=sinCache1b[j];
              costemp1=cosCache1b[j];
              switch(qobj->normals)
              {
                 case GLU_FLAT:
                 case GLU_SMOOTH:
                      sintemp2=sinCache2b[j];
                      costemp2=cosCache2b[j];
                      break;
                 default:
                      break;
              }

              for (i=0; i<slices; i++)
              {
                 switch(qobj->normals)
                 {
                    case GLU_FLAT:
                    case GLU_SMOOTH:
                         normals[i][0]=sinCache2a[i]*sintemp2;
                         normals[i][1]=cosCache2a[i]*sintemp2;
                         normals[i][2]=costemp2;
                         break;
                    case GLU_NONE:
                    default:
                         break;
                 }
                 zLow=j*radius/stacks;
                 if (qobj->textureCoords)
                 {
                    texcoords[i][0]=1-(GLfloat)i/slices;
                    texcoords[i][1]=1-(GLfloat)j/stacks;
                 }
                 vertices[i][0]=sintemp1*sinCache1a[i];
                 vertices[i][1]=sintemp1*cosCache1a[i];
                 vertices[i][2]=costemp1;
              }
              glDrawArrays(GL_POINTS, 0, slices);
           }
           break;
      case GLU_LINE:
      case GLU_SILHOUETTE:
           for (j=1; j<stacks; j++)
           {
              sintemp1=sinCache1b[j];
              costemp1=cosCache1b[j];
              switch(qobj->normals)
              {
                 case GLU_FLAT:
                 case GLU_SMOOTH:
                      sintemp2=sinCache2b[j];
                      costemp2=cosCache2b[j];
                      break;
                 default:
                      break;
              }

              for (i=0; i<=slices; i++)
              {
                 switch(qobj->normals)
                 {
                    case GLU_FLAT:
                         normals[i][0]=sinCache3a[i]*sintemp2;
                         normals[i][1]=cosCache3a[i]*sintemp2;
                         normals[i][2]=costemp2;
                         break;
                    case GLU_SMOOTH:
                         normals[i][0]=sinCache2a[i]*sintemp2;
                         normals[i][1]=cosCache2a[i]*sintemp2;
                         normals[i][2]=costemp2;
                         break;
                    case GLU_NONE:
                    default:
                         break;
                 }
                 if (qobj->textureCoords)
                 {
                    texcoords[i][0]=1-(GLfloat)i/slices;
                    texcoords[i][1]=1-(GLfloat)j/stacks;
                 }
                 vertices[i][0]=sintemp1*sinCache1a[i];
                 vertices[i][1]=sintemp1*cosCache1a[i];
                 vertices[i][2]=costemp1;
              }
              glDrawArrays(GL_LINE_STRIP, 0, slices+1);
           }

           for (i=0; i<slices; i++)
           {
              sintemp1=sinCache1a[i];
              costemp1=cosCache1a[i];
              switch(qobj->normals)
              {
                 case GLU_FLAT:
                 case GLU_SMOOTH:
                      sintemp2=sinCache2a[i];
                      costemp2=cosCache2a[i];
                      break;
                 default:
                      break;
              }

              for (j=0; j<=stacks; j++)
              {
                 switch(qobj->normals)
                 {
                    case GLU_FLAT:
                         normals[j][0]=sintemp2*sinCache3b[j];
                         normals[j][1]=costemp2*sinCache3b[j];
                         normals[j][2]=cosCache3b[j];
                         break;
                    case GLU_SMOOTH:
                         normals[j][0]=sintemp2*sinCache2b[j];
                         normals[j][1]=costemp2*sinCache2b[j];
                         normals[j][2]=cosCache2b[j];
                         break;
                    case GLU_NONE:
                    default:
                         break;
                 }

                 if (qobj->textureCoords)
                 {
                    texcoords[j][0]=1-(GLfloat)i/slices;
                    texcoords[j][1]=1-(GLfloat)j/stacks;
                 }
                 vertices[j][0]=sintemp1*sinCache1b[j];
                 vertices[j][1]=costemp1*sinCache1b[j];
                 vertices[j][2]=cosCache1b[j];
              }
              glDrawArrays(GL_LINE_STRIP, 0, stacks+1);
           }
           break;
      default:
           break;
    }

   /* Disable or re-enable arrays */
   if (vertex_enabled)
   {
      /* Re-enable vertex array */
      glEnableClientState(GL_VERTEX_ARRAY);
   }
   else
   {
      glDisableClientState(GL_VERTEX_ARRAY);
   }

   if (texcoord_enabled)
   {
      glEnableClientState(GL_TEXTURE_COORD_ARRAY);
   }
   else
   {
      glDisableClientState(GL_TEXTURE_COORD_ARRAY);
   }

   if (normal_enabled)
   {
      glEnableClientState(GL_NORMAL_ARRAY);
   }
   else
   {
      glDisableClientState(GL_NORMAL_ARRAY);
   }

   if (color_enabled)
   {
      glEnableClientState(GL_COLOR_ARRAY);
   }
   else
   {
      glDisableClientState(GL_COLOR_ARRAY);
   }
}
