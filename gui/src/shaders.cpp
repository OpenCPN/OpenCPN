/***************************************************************************
 *
 * Project:  OpenCPN
 *
 ***************************************************************************
 *   Copyright (C) 2017 by David S. Register                               *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 **************************************************************************/

#include "shaders.h"

#if defined(USE_ANDROID_GLES2) || defined(ocpnUSE_GLSL)

#ifdef USE_ANDROID_GLES2
#include <GLES2/gl2.h>
#include "qdebug.h"
#endif

#ifdef USE_ANDROID_GLES2
const GLchar *preamble = "\n";
#else
const GLchar *preamble =
    "#version 120\n"
    "#define precision\n"
    "#define lowp\n"
    "#define mediump\n"
    "#define highp\n";
#endif

// Simple colored triangle shader

static const GLchar *color_tri_vertex_shader_source =
    "attribute vec2 position;\n"
    "uniform mat4 MVMatrix;\n"
    "uniform mat4 TransformMatrix;\n"
    "uniform vec4 color;\n"
    "varying vec4 fragColor;\n"
    "void main() {\n"
    "   fragColor = color;\n"
    "   gl_Position = MVMatrix * TransformMatrix * vec4(position, 0.0, 1.0);\n"
    "}\n";

static const GLchar *color_tri_fragment_shader_source =
    "precision lowp float;\n"
    "varying vec4 fragColor;\n"
    "void main() {\n"
    "   gl_FragColor = fragColor;\n"
    "}\n";

// Simple 2D texture shader
static const GLchar *texture_2D_vertex_shader_source =
    "attribute vec2 aPos;\n"
    "attribute vec2 aUV;\n"
    "uniform mat4 MVMatrix;\n"
    "uniform mat4 TransformMatrix;\n"
    "varying vec2 varCoord;\n"
    "void main() {\n"
    "   gl_Position = MVMatrix * TransformMatrix * vec4(aPos, 0.0, 1.0);\n"
    "   varCoord = aUV;\n"
    "}\n";

static const GLchar *texture_2D_fragment_shader_source =
    "precision lowp float;\n"
    "uniform sampler2D uTex;\n"
    "varying vec2 varCoord;\n"
    "void main() {\n"
    "   gl_FragColor = texture2D(uTex, varCoord);\n"
    "}\n";

//  Circle shader

static const GLchar *circle_filled_vertex_shader_source =
    "precision highp float;\n"
    "attribute vec2 aPos;\n"
    "uniform mat4 MVMatrix;\n"
    "uniform mat4 TransformMatrix;\n"
    "void main() {\n"
    "   gl_Position = MVMatrix * TransformMatrix * vec4(aPos, 0.0, 1.0);\n"
    "}\n";

static const GLchar *circle_filled_fragment_shader_source =
    "precision highp float;\n"
    "uniform float border_width;\n"
    "uniform float circle_radius;\n"
    "uniform vec4 circle_color;\n"
    "uniform vec4 border_color;\n"
    "uniform vec2 circle_center;\n"
    "void main(){\n"
    "float d = distance(gl_FragCoord.xy, circle_center);\n"
    "if (d < (circle_radius - border_width)) { gl_FragColor = circle_color; }\n"
    "else if (d < circle_radius) { gl_FragColor = border_color; }\n"
    "else { gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0); }\n"
    "}\n";

// Alpha 2D texture shader
static const GLchar *texture_2DA_vertex_shader_source =
    "attribute vec2 aPos;\n"
    "attribute vec2 aUV;\n"
    "uniform mat4 MVMatrix;\n"
    "uniform mat4 TransformMatrix;\n"
    "varying vec2 varCoord;\n"
    "void main() {\n"
    "   gl_Position = MVMatrix * TransformMatrix * vec4(aPos, 0.0, 1.0);\n"
    "   varCoord = aUV;\n"
    "}\n";

static const GLchar *texture_2DA_fragment_shader_source =
    "precision lowp float;\n"
    "uniform sampler2D uTex;\n"
    "varying vec2 varCoord;\n"
    "uniform vec4 color;\n"
    "void main() {\n"
    "   gl_FragColor = texture2D(uTex, varCoord) + color;\n"
    "}\n";

// https://vitaliburkov.wordpress.com/2016/09/17/simple-and-fast-high-quality-antialiased-lines-with-opengl/
static const GLchar *AALine_vertex_shader_source =
    "uniform vec2 uViewPort; //Width and Height of the viewport\n"
    "varying vec2 vLineCenter;\n"
    "attribute vec2 position;\n"
    "uniform mat4 MVMatrix;\n"
    "uniform mat4 TransformMatrix;\n"
    "void main()\n"
    "{\n"
    "   vec4 pp = MVMatrix * vec4(position, 0.0, 1.0);\n"
    "   gl_Position = pp;\n"
    "   vec2 vp = uViewPort;\n"
    "   vLineCenter = 0.5*(pp.xy + vec2(1, 1))*vp;\n"
    "}\n";

static const GLchar *AALine_fragment_shader_source =
    "precision mediump float;\n"
    "uniform float uLineWidth;\n"
    "uniform vec4 color;\n"
    "uniform float uBlendFactor; //1.5..2.5\n"
    "varying vec2 vLineCenter;\n"
    "void main()\n"
    "{\n"
    "    vec4 col = color;\n"
    "    float d = length(vLineCenter-gl_FragCoord.xy);\n"
    "    float w = uLineWidth;\n"
    "    if (d>w)\n"
    "      col.w = 0.0;\n"
    "    else{\n"
    "      if(float((w/2.0-d)/(w/2.0)) < .5){\n"
    "        //col.w *= pow(float((w-d)/w), uBlendFactor);\n"
    "        col.w *= pow(float((w/2.0-d)/(w/2.0)), uBlendFactor);\n"
    "      }\n"
    "    }\n"
    "    gl_FragColor = col;\n"
    "}\n";

//  Ring shader

static const GLchar *ring_vertex_shader_source =
    "precision highp float;\n"
    "attribute vec2 aPos;\n"
    "uniform mat4 MVMatrix;\n"
    "uniform mat4 TransformMatrix;\n"
    "void main() {\n"
    "   gl_Position = MVMatrix * TransformMatrix * vec4(aPos, 0.0, 1.0);\n"
    "}\n";

static const GLchar *ring_fragment_shader_source =
    "precision highp float;\n"
    "uniform float border_width;\n"
    "uniform float circle_radius;\n"
    "uniform float ring_width;\n"
    "uniform vec4 circle_color;\n"
    "uniform vec4 border_color;\n"
    "uniform vec2 circle_center;\n"
    "uniform float sector_1;\n"
    "uniform float sector_2;\n"

    "void main(){\n"
    "const float PI = 3.14159265358979323846264;\n"
    "bool bdraw = false;\n"

    "float angle = atan(gl_FragCoord.y-circle_center.y, "
    "gl_FragCoord.x-circle_center.x);\n"
    "angle = PI/2.0 - angle;\n"
    "if(angle < 0.0) angle += PI * 2.0;\n"

    "if(sector_2 > PI * 2.0){\n"
    "    if((angle > sector_1) && (angle < (PI * 2.0) )){\n"
    "        bdraw = true;\n"
    "    }\n"
    "    if(angle < sector_2 - (PI * 2.0)){\n"
    "        bdraw = true;\n"
    "    }\n"
    "} else {\n"
    "    if((angle > sector_1) && (angle < sector_2)){\n"
    "        bdraw = true;\n"
    "    }\n"
    "}\n"

    "if(bdraw){\n"
    "   float d = distance(gl_FragCoord.xy, circle_center);\n"
    "   if (d > circle_radius) {\n"
    "       discard;\n"
    "   } else if( d > (circle_radius - border_width)) {\n"
    "       gl_FragColor = border_color;\n"
    "   } else if( d > (circle_radius - border_width - ring_width)) {\n"
    "       gl_FragColor = circle_color;\n"
    "   } else if( d > (circle_radius - border_width - ring_width - "
    "border_width)) {\n"
    "       gl_FragColor = border_color;\n"
    "   } else  {\n"
    "       discard;\n"
    "   }\n"
    "} else{\n"
    "   discard;\n"
    "}\n"
    "}\n";

// Alpha 2D texture shader
static const GLchar *Android_texture_2DA_vertex_shader_source =
    "attribute vec2 aPos;\n"
    "attribute vec2 aUV;\n"
    "uniform mat4 MVMatrix;\n"
    "uniform mat4 TransformMatrix;\n"
    "varying vec2 varCoord;\n"
    "void main() {\n"
    "   gl_Position = MVMatrix * TransformMatrix * vec4(aPos, 0.0, 1.0);\n"
    "   varCoord = aUV;\n"
    "}\n";

static const GLchar *Android_texture_2DA_fragment_shader_source =
    "precision lowp float;\n"
    "uniform sampler2D uTex;\n"
    "varying vec2 varCoord;\n"
    "uniform vec4 color;\n"
    "void main() {\n"
    "   gl_FragColor = texture2D(uTex, varCoord) + color;\n"
    "}\n";

GLShaderProgram *pAALine_shader_program[2];
GLShaderProgram *pcolor_tri_shader_program[2];
GLShaderProgram *ptexture_2D_shader_program[2];
GLShaderProgram *pcircle_filled_shader_program[2];
GLShaderProgram *ptexture_2DA_shader_program[2];
GLShaderProgram *pring_shader_program[2];

GLint texture_2DA_vertex_shader_p;
GLint texture_2DA_fragment_shader_p;
GLint texture_2DA_shader_program;

bool bShadersLoaded[2];

bool loadShaders(int index) {
  // Are the shaders ready?
  if (bShadersLoaded[index]) {
    reConfigureShaders(index);
    return true;
  }

  bool ret_val = true;
  GLint success;

  // Simple colored triangle shader
  if (!pcolor_tri_shader_program[index]) {
    GLShaderProgram *shaderProgram = new GLShaderProgram;
    shaderProgram->addShaderFromSource(color_tri_vertex_shader_source,
                                       GL_VERTEX_SHADER);
    shaderProgram->addShaderFromSource(color_tri_fragment_shader_source,
                                       GL_FRAGMENT_SHADER);
    shaderProgram->linkProgram();

    if (shaderProgram->isOK()) pcolor_tri_shader_program[index] = shaderProgram;
  }

  if (!ptexture_2D_shader_program[index]) {
    GLShaderProgram *shaderProgram = new GLShaderProgram;
    shaderProgram->addShaderFromSource(texture_2D_vertex_shader_source,
                                       GL_VERTEX_SHADER);
    shaderProgram->addShaderFromSource(texture_2D_fragment_shader_source,
                                       GL_FRAGMENT_SHADER);
    shaderProgram->linkProgram();

    if (shaderProgram->isOK())
      ptexture_2D_shader_program[index] = shaderProgram;
  }

  if (!pcircle_filled_shader_program[index]) {
    GLShaderProgram *shaderProgram = new GLShaderProgram;
    shaderProgram->addShaderFromSource(circle_filled_vertex_shader_source,
                                       GL_VERTEX_SHADER);
    shaderProgram->addShaderFromSource(circle_filled_fragment_shader_source,
                                       GL_FRAGMENT_SHADER);
    shaderProgram->linkProgram();

    if (shaderProgram->isOK())
      pcircle_filled_shader_program[index] = shaderProgram;
  }

  if (!ptexture_2DA_shader_program[index]) {
    GLShaderProgram *shaderProgram = new GLShaderProgram;
    shaderProgram->addShaderFromSource(texture_2DA_vertex_shader_source,
                                       GL_VERTEX_SHADER);
    shaderProgram->addShaderFromSource(texture_2DA_fragment_shader_source,
                                       GL_FRAGMENT_SHADER);
    shaderProgram->linkProgram();

    if (shaderProgram->isOK())
      ptexture_2DA_shader_program[index] = shaderProgram;
  }

  if (!pAALine_shader_program[index]) {
    GLShaderProgram *shaderProgram = new GLShaderProgram;
    shaderProgram->addShaderFromSource(AALine_fragment_shader_source,
                                       GL_FRAGMENT_SHADER);
    shaderProgram->addShaderFromSource(AALine_vertex_shader_source,
                                       GL_VERTEX_SHADER);
    shaderProgram->linkProgram();

    if (shaderProgram->isOK()) pAALine_shader_program[index] = shaderProgram;
  }

  // ring shader
  if (!pring_shader_program[index]) {
    GLShaderProgram *shaderProgram = new GLShaderProgram;
    shaderProgram->addShaderFromSource(ring_fragment_shader_source,
                                       GL_FRAGMENT_SHADER);
    shaderProgram->addShaderFromSource(ring_vertex_shader_source,
                                       GL_VERTEX_SHADER);
    shaderProgram->linkProgram();

    if (shaderProgram->isOK()) pring_shader_program[index] = shaderProgram;
  }

#ifdef __ANDROID__
  //  2DA shader called by some Android plugins
  if (!texture_2DA_vertex_shader_p) {
    /* Vertex shader */
    texture_2DA_vertex_shader_p = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(texture_2DA_vertex_shader_p, 1,
                   &Android_texture_2DA_vertex_shader_source, NULL);
    glCompileShader(texture_2DA_vertex_shader_p);
    glGetShaderiv(texture_2DA_vertex_shader_p, GL_COMPILE_STATUS, &success);
    if (!success) {
      // glGetShaderInfoLog(texture_2DA_vertex_shader_p, INFOLOG_LEN, NULL,
      //                    infoLog);
      // printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
      ret_val = false;
    }
  }

  if (!texture_2DA_fragment_shader_p) {
    /* Fragment shader */
    texture_2DA_fragment_shader_p = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(texture_2DA_fragment_shader_p, 1,
                   &Android_texture_2DA_fragment_shader_source, NULL);
    glCompileShader(texture_2DA_fragment_shader_p);
    glGetShaderiv(texture_2DA_fragment_shader_p, GL_COMPILE_STATUS, &success);
    if (!success) {
      // glGetShaderInfoLog(texture_2DA_fragment_shader_p, INFOLOG_LEN,
      //                    NULL, infoLog);
      // printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
      ret_val = false;
    }
  }

  if (!texture_2DA_shader_program) {
    /* Link shaders */
    texture_2DA_shader_program = glCreateProgram();
    glAttachShader(texture_2DA_shader_program, texture_2DA_vertex_shader_p);
    glAttachShader(texture_2DA_shader_program, texture_2DA_fragment_shader_p);
    glLinkProgram(texture_2DA_shader_program);
    glGetProgramiv(texture_2DA_shader_program, GL_LINK_STATUS, &success);
    if (!success) {
      // glGetProgramInfoLog(texture_2DA_shader_program, INFOLOG_LEN,
      //                     NULL, infoLog);
      // printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
      ret_val = false;
    }
  }
#endif

  bShadersLoaded[index] = true;
  reConfigureShaders(index);

  return ret_val;
}

void reConfigureShaders(int index) {}

void unloadShaders() { bShadersLoaded[0] = bShadersLoaded[1] = false; }

GLShaderProgram *GetStaticTriShader() {
  GLShaderProgram *shaderProgram = new GLShaderProgram;
  shaderProgram->addShaderFromSource(color_tri_vertex_shader_source,
                                     GL_VERTEX_SHADER);
  shaderProgram->addShaderFromSource(color_tri_fragment_shader_source,
                                     GL_FRAGMENT_SHADER);
  shaderProgram->linkProgram();

  if (shaderProgram->isOK())
    return shaderProgram;
  else
    return NULL;
}

#endif
