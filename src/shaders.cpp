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

#ifdef USE_ANDROID_GLES2
#include "qdebug.h"

#include <GLES2/gl2.h>

// Simple colored triangle shader

static const GLchar* color_tri_vertex_shader_source =
    "attribute vec2 position;\n"
    "uniform mat4 MVMatrix;\n"
    "uniform mat4 TransformMatrix;\n"
    "uniform vec4 color;\n"
    "varying vec4 fragColor;\n"
    "void main() {\n"
    "   fragColor = color;\n"
    "   gl_Position = MVMatrix * TransformMatrix * vec4(position, 0.0, 1.0);\n"
    "}\n";

static const GLchar* color_tri_fragment_shader_source =
    "precision lowp float;\n"
    "varying vec4 fragColor;\n"
    "void main() {\n"
    "   gl_FragColor = fragColor;\n"
    "}\n";

// Simple 2D texture shader
static const GLchar* texture_2D_vertex_shader_source =
    "attribute vec2 aPos;\n"
    "attribute vec2 aUV;\n"
    "uniform mat4 MVMatrix;\n"
    "uniform mat4 TransformMatrix;\n"
    "varying vec2 varCoord;\n"
    "void main() {\n"
    "   gl_Position = MVMatrix * TransformMatrix * vec4(aPos, 0.0, 1.0);\n"
    "   varCoord = aUV;\n"
    "}\n";

static const GLchar* texture_2D_fragment_shader_source =
    "precision lowp float;\n"
    "uniform sampler2D uTex;\n"
    "varying vec2 varCoord;\n"
    "void main() {\n"
    "   gl_FragColor = texture2D(uTex, varCoord);\n"
    "}\n";

// Fade Texture shader
static const GLchar* fade_texture_2D_vertex_shader_source =
    "precision highp float;\n"
    "attribute vec2 aPos;\n"
    "attribute vec2 aUV;\n"
    "attribute vec2 aUV2;\n"
    "uniform mat4 MVMatrix;\n"
    "varying vec2 varCoord;\n"
    "varying vec2 varCoord2;\n"
    "void main() {\n"
    "   gl_Position = MVMatrix * vec4(aPos, 0.0, 1.0);\n"
    "   varCoord = aUV.st;\n"
    "   varCoord2 = aUV2.st;\n"
    "}\n";

static const GLchar* fade_texture_2D_fragment_shader_source =
    "precision highp float;\n"
    "uniform sampler2D uTex;\n"
    "uniform sampler2D uTex2;\n"
    "uniform lowp float texAlpha;\n"
    "varying vec2 varCoord;\n"
    "varying vec2 varCoord2;\n"
    "void main() {\n"
    "   mediump vec4 texColor = texture2D(uTex, varCoord);\n"
    "   mediump vec4 texTwoColor = texture2D(uTex2, varCoord2);\n"
    "   gl_FragColor = ((texTwoColor * (1.0 - texAlpha)) + (texColor * "
    "texAlpha));\n"
    "}\n";

//  Circle shader

static const GLchar* circle_filled_vertex_shader_source =
    "precision highp float;\n"
    "attribute vec2 aPos;\n"
    "uniform mat4 MVMatrix;\n"
    "uniform mat4 TransformMatrix;\n"
    "void main() {\n"
    "   gl_Position = MVMatrix * TransformMatrix * vec4(aPos, 0.0, 1.0);\n"
    "}\n";

static const GLchar* circle_filled_fragment_shader_source =
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

//  2D texture shader for FBOs
static const GLchar* FBO_texture_2D_vertex_shader_source =
    "attribute vec2 aPos;\n"
    "attribute vec2 aUV;\n"
    "uniform mat4 MVMatrix;\n"
    "varying vec2 varCoord;\n"
    "void main() {\n"
    "   gl_Position = MVMatrix * vec4(aPos, 0.0, 1.0);\n"
    "   varCoord = aUV;\n"
    "}\n";

static const GLchar* FBO_texture_2D_fragment_shader_source =
    "precision lowp float;\n"
    "uniform sampler2D uTex;\n"
    "varying vec2 varCoord;\n"
    "void main() {\n"
    "   gl_FragColor = texture2D(uTex, varCoord);\n"
    "}\n";

// Alpah 2D texture shader
static const GLchar* texture_2DA_vertex_shader_source =
    "attribute vec2 aPos;\n"
    "attribute vec2 aUV;\n"
    "uniform mat4 MVMatrix;\n"
    "uniform mat4 TransformMatrix;\n"
    "varying vec2 varCoord;\n"
    "void main() {\n"
    "   gl_Position = MVMatrix * TransformMatrix * vec4(aPos, 0.0, 1.0);\n"
    "   varCoord = aUV;\n"
    "}\n";

static const GLchar* texture_2DA_fragment_shader_source =
    "precision lowp float;\n"
    "uniform sampler2D uTex;\n"
    "varying vec2 varCoord;\n"
    "uniform vec4 color;\n"
    "void main() {\n"
    "   gl_FragColor = texture2D(uTex, varCoord) + color;\n"
    "}\n";

GLint color_tri_fragment_shader;
GLint color_tri_shader_program;
GLint color_tri_vertex_shader;

GLint texture_2D_fragment_shader;
GLint texture_2D_shader_program;
GLint texture_2D_vertex_shader;

GLint fade_texture_2D_fragment_shader;
GLint fade_texture_2D_shader_program;
GLint fade_texture_2D_vertex_shader;

GLint circle_filled_shader_program;
GLint circle_filled_vertex_shader;
GLint circle_filled_fragment_shader;

GLint FBO_texture_2D_fragment_shader;
GLint FBO_texture_2D_shader_program;
GLint FBO_texture_2D_vertex_shader;

GLint texture_2DA_fragment_shader;
GLint texture_2DA_shader_program;
GLint texture_2DA_vertex_shader;

// Protos
GLint color_tri_fragment_shader_p[2];
GLint color_tri_shader_program_p[2];
GLint color_tri_vertex_shader_p[2];

GLint texture_2D_fragment_shader_p[2];
GLint texture_2D_shader_program_p[2];
GLint texture_2D_vertex_shader_p[2];

GLint fade_texture_2D_fragment_shader_p[2];
GLint fade_texture_2D_shader_program_p[2];
GLint fade_texture_2D_vertex_shader_p[2];

GLint circle_filled_shader_program_p[2];
GLint circle_filled_vertex_shader_p[2];
GLint circle_filled_fragment_shader_p[2];

GLint FBO_texture_2D_fragment_shader_p[2];
GLint FBO_texture_2D_shader_program_p[2];
GLint FBO_texture_2D_vertex_shader_p[2];

GLint texture_2DA_fragment_shader_p[2];
GLint texture_2DA_shader_program_p[2];
GLint texture_2DA_vertex_shader_p[2];

bool bShadersLoaded[2];

bool loadShaders(int index) {
  if (bShadersLoaded[index]) {
    reConfigureShaders(index);
    return true;
  }

  bool ret_val = true;
  GLint success;

  enum Consts { INFOLOG_LEN = 512 };
  GLchar infoLog[INFOLOG_LEN];

  // Are the shaders ready?

  // Simple colored triangle shader

  if (!color_tri_vertex_shader_p[index]) {
    /* Vertex shader */
    color_tri_vertex_shader_p[index] = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(color_tri_vertex_shader_p[index], 1,
                   &color_tri_vertex_shader_source, NULL);
    glCompileShader(color_tri_vertex_shader_p[index]);
    glGetShaderiv(color_tri_vertex_shader_p[index], GL_COMPILE_STATUS,
                  &success);
    if (!success) {
      glGetShaderInfoLog(color_tri_vertex_shader_p[index], INFOLOG_LEN, NULL,
                         infoLog);
      printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
      ret_val = false;
    }
  }

  if (!color_tri_fragment_shader_p[index]) {
    /* Fragment shader */
    color_tri_fragment_shader_p[index] = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(color_tri_fragment_shader_p[index], 1,
                   &color_tri_fragment_shader_source, NULL);
    glCompileShader(color_tri_fragment_shader_p[index]);
    glGetShaderiv(color_tri_fragment_shader_p[index], GL_COMPILE_STATUS,
                  &success);
    if (!success) {
      glGetShaderInfoLog(color_tri_fragment_shader_p[index], INFOLOG_LEN, NULL,
                         infoLog);
      printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
      ret_val = false;
    }
  }

  if (!color_tri_shader_program_p[index]) {
    /* Link shaders */
    color_tri_shader_program_p[index] = glCreateProgram();
    glAttachShader(color_tri_shader_program_p[index],
                   color_tri_fragment_shader_p[index]);
    glAttachShader(color_tri_shader_program_p[index],
                   color_tri_vertex_shader_p[index]);
    glLinkProgram(color_tri_shader_program_p[index]);
    glGetProgramiv(color_tri_shader_program_p[index], GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(color_tri_shader_program_p[index], INFOLOG_LEN, NULL,
                          infoLog);
      printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
      ret_val = false;
    }
  }

  // Simple 2D texture shader

  if (!texture_2D_vertex_shader_p[index]) {
    /* Vertex shader */
    texture_2D_vertex_shader_p[index] = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(texture_2D_vertex_shader_p[index], 1,
                   &texture_2D_vertex_shader_source, NULL);
    glCompileShader(texture_2D_vertex_shader_p[index]);
    glGetShaderiv(texture_2D_vertex_shader_p[index], GL_COMPILE_STATUS,
                  &success);
    if (!success) {
      glGetShaderInfoLog(texture_2D_vertex_shader_p[index], INFOLOG_LEN, NULL,
                         infoLog);
      printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
      ret_val = false;
    }
  }

  if (!texture_2D_fragment_shader_p[index]) {
    /* Fragment shader */
    texture_2D_fragment_shader_p[index] = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(texture_2D_fragment_shader_p[index], 1,
                   &texture_2D_fragment_shader_source, NULL);
    glCompileShader(texture_2D_fragment_shader_p[index]);
    glGetShaderiv(texture_2D_fragment_shader_p[index], GL_COMPILE_STATUS,
                  &success);
    if (!success) {
      glGetShaderInfoLog(texture_2D_fragment_shader_p[index], INFOLOG_LEN, NULL,
                         infoLog);
      printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
      ret_val = false;
    }
  }

  if (!texture_2D_shader_program_p[index]) {
    /* Link shaders */
    texture_2D_shader_program_p[index] = glCreateProgram();
    glAttachShader(texture_2D_shader_program_p[index],
                   texture_2D_vertex_shader_p[index]);
    glAttachShader(texture_2D_shader_program_p[index],
                   texture_2D_fragment_shader_p[index]);
    glLinkProgram(texture_2D_shader_program_p[index]);
    glGetProgramiv(texture_2D_shader_program_p[index], GL_LINK_STATUS,
                   &success);
    if (!success) {
      glGetProgramInfoLog(texture_2D_shader_program_p[index], INFOLOG_LEN, NULL,
                          infoLog);
      printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
      ret_val = false;
    }
  }

  // Fade texture shader
  if (!fade_texture_2D_vertex_shader_p[index]) {
    /* Vertex shader */
    fade_texture_2D_vertex_shader_p[index] = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(fade_texture_2D_vertex_shader_p[index], 1,
                   &fade_texture_2D_vertex_shader_source, NULL);
    glCompileShader(fade_texture_2D_vertex_shader_p[index]);
    glGetShaderiv(fade_texture_2D_vertex_shader_p[index], GL_COMPILE_STATUS,
                  &success);
    if (!success) {
      glGetShaderInfoLog(fade_texture_2D_vertex_shader_p[index], INFOLOG_LEN,
                         NULL, infoLog);
      printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
      ret_val = false;
    }
  }

  if (!fade_texture_2D_fragment_shader_p[index]) {
    /* Fragment shader */
    fade_texture_2D_fragment_shader_p[index] =
        glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fade_texture_2D_fragment_shader_p[index], 1,
                   &fade_texture_2D_fragment_shader_source, NULL);
    glCompileShader(fade_texture_2D_fragment_shader_p[index]);
    glGetShaderiv(fade_texture_2D_fragment_shader_p[index], GL_COMPILE_STATUS,
                  &success);
    if (!success) {
      glGetShaderInfoLog(fade_texture_2D_fragment_shader_p[index], INFOLOG_LEN,
                         NULL, infoLog);
      printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
      ret_val = false;
    }
  }

  if (!fade_texture_2D_shader_program_p[index]) {
    /* Link shaders */
    fade_texture_2D_shader_program_p[index] = glCreateProgram();
    glAttachShader(fade_texture_2D_shader_program_p[index],
                   fade_texture_2D_vertex_shader_p[index]);
    glAttachShader(fade_texture_2D_shader_program_p[index],
                   fade_texture_2D_fragment_shader_p[index]);
    glLinkProgram(fade_texture_2D_shader_program_p[index]);
    glGetProgramiv(fade_texture_2D_shader_program_p[index], GL_LINK_STATUS,
                   &success);
    if (!success) {
      glGetProgramInfoLog(fade_texture_2D_shader_program_p[index], INFOLOG_LEN,
                          NULL, infoLog);
      printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
      ret_val = false;
    }
  }

  // Circle shader
  if (!circle_filled_vertex_shader_p[index]) {
    /* Vertex shader */
    circle_filled_vertex_shader_p[index] = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(circle_filled_vertex_shader_p[index], 1,
                   &circle_filled_vertex_shader_source, NULL);
    glCompileShader(circle_filled_vertex_shader_p[index]);
    glGetShaderiv(circle_filled_vertex_shader_p[index], GL_COMPILE_STATUS,
                  &success);
    if (!success) {
      glGetShaderInfoLog(circle_filled_vertex_shader_p[index], INFOLOG_LEN,
                         NULL, infoLog);
      printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
      qDebug() << infoLog;
      ret_val = false;
    }
  }

  if (!circle_filled_fragment_shader_p[index]) {
    /* Fragment shader */
    circle_filled_fragment_shader_p[index] = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(circle_filled_fragment_shader_p[index], 1,
                   &circle_filled_fragment_shader_source, NULL);
    glCompileShader(circle_filled_fragment_shader_p[index]);
    glGetShaderiv(circle_filled_fragment_shader_p[index], GL_COMPILE_STATUS,
                  &success);
    if (!success) {
      glGetShaderInfoLog(circle_filled_fragment_shader_p[index], INFOLOG_LEN,
                         NULL, infoLog);
      printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
      qDebug() << infoLog;
      ret_val = false;
    }
  }

  if (!circle_filled_shader_program_p[index]) {
    /* Link shaders */
    circle_filled_shader_program_p[index] = glCreateProgram();
    glAttachShader(circle_filled_shader_program_p[index],
                   circle_filled_vertex_shader_p[index]);
    glAttachShader(circle_filled_shader_program_p[index],
                   circle_filled_fragment_shader_p[index]);
    glLinkProgram(circle_filled_shader_program_p[index]);
    glGetProgramiv(circle_filled_shader_program_p[index], GL_LINK_STATUS,
                   &success);
    if (!success) {
      glGetProgramInfoLog(circle_filled_shader_program_p[index], INFOLOG_LEN,
                          NULL, infoLog);
      printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
      qDebug() << infoLog;
      ret_val = false;
    }
  }

  // FBO 2D texture shader

  if (!FBO_texture_2D_vertex_shader_p[index]) {
    /* Vertex shader */
    FBO_texture_2D_vertex_shader_p[index] = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(FBO_texture_2D_vertex_shader_p[index], 1,
                   &FBO_texture_2D_vertex_shader_source, NULL);
    glCompileShader(FBO_texture_2D_vertex_shader_p[index]);
    glGetShaderiv(FBO_texture_2D_vertex_shader_p[index], GL_COMPILE_STATUS,
                  &success);
    if (!success) {
      glGetShaderInfoLog(FBO_texture_2D_vertex_shader_p[index], INFOLOG_LEN,
                         NULL, infoLog);
      printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
      ret_val = false;
    }
  }

  if (!FBO_texture_2D_fragment_shader_p[index]) {
    /* Fragment shader */
    FBO_texture_2D_fragment_shader_p[index] =
        glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(FBO_texture_2D_fragment_shader_p[index], 1,
                   &FBO_texture_2D_fragment_shader_source, NULL);
    glCompileShader(FBO_texture_2D_fragment_shader_p[index]);
    glGetShaderiv(FBO_texture_2D_fragment_shader_p[index], GL_COMPILE_STATUS,
                  &success);
    if (!success) {
      glGetShaderInfoLog(FBO_texture_2D_fragment_shader_p[index], INFOLOG_LEN,
                         NULL, infoLog);
      printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
      ret_val = false;
    }
  }

  if (!FBO_texture_2D_shader_program_p[index]) {
    /* Link shaders */
    FBO_texture_2D_shader_program_p[index] = glCreateProgram();
    glAttachShader(FBO_texture_2D_shader_program_p[index],
                   FBO_texture_2D_vertex_shader_p[index]);
    glAttachShader(FBO_texture_2D_shader_program_p[index],
                   FBO_texture_2D_fragment_shader_p[index]);
    glLinkProgram(FBO_texture_2D_shader_program_p[index]);
    glGetProgramiv(FBO_texture_2D_shader_program_p[index], GL_LINK_STATUS,
                   &success);
    if (!success) {
      glGetProgramInfoLog(FBO_texture_2D_shader_program_p[index], INFOLOG_LEN,
                          NULL, infoLog);
      printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
      ret_val = false;
    }
  }

  // 2D Alpha color texture shader

  if (!texture_2DA_vertex_shader_p[index]) {
    /* Vertex shader */
    texture_2DA_vertex_shader_p[index] = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(texture_2DA_vertex_shader_p[index], 1,
                   &texture_2DA_vertex_shader_source, NULL);
    glCompileShader(texture_2DA_vertex_shader_p[index]);
    glGetShaderiv(texture_2DA_vertex_shader_p[index], GL_COMPILE_STATUS,
                  &success);
    if (!success) {
      glGetShaderInfoLog(texture_2DA_vertex_shader_p[index], INFOLOG_LEN, NULL,
                         infoLog);
      printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
      qDebug() << infoLog;
      ret_val = false;
    }
  }

  if (!texture_2DA_fragment_shader_p[index]) {
    /* Fragment shader */
    texture_2DA_fragment_shader_p[index] = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(texture_2DA_fragment_shader_p[index], 1,
                   &texture_2DA_fragment_shader_source, NULL);
    glCompileShader(texture_2DA_fragment_shader_p[index]);
    glGetShaderiv(texture_2DA_fragment_shader_p[index], GL_COMPILE_STATUS,
                  &success);
    if (!success) {
      glGetShaderInfoLog(texture_2DA_fragment_shader_p[index], INFOLOG_LEN,
                         NULL, infoLog);
      printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
      qDebug() << infoLog;
      ret_val = false;
    }
  }

  if (!texture_2DA_shader_program_p[index]) {
    /* Link shaders */
    texture_2DA_shader_program_p[index] = glCreateProgram();
    glAttachShader(texture_2DA_shader_program_p[index],
                   texture_2DA_vertex_shader_p[index]);
    glAttachShader(texture_2DA_shader_program_p[index],
                   texture_2DA_fragment_shader_p[index]);
    glLinkProgram(texture_2DA_shader_program_p[index]);
    glGetProgramiv(texture_2DA_shader_program_p[index], GL_LINK_STATUS,
                   &success);
    if (!success) {
      glGetProgramInfoLog(texture_2DA_shader_program_p[index], INFOLOG_LEN,
                          NULL, infoLog);
      printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
      qDebug() << infoLog;
      ret_val = false;
    }
  }

  qDebug() << "Shader Load " << ret_val;

  bShadersLoaded[index] = true;
  reConfigureShaders(index);

  return ret_val;
}

void reConfigureShaders(int index) {
  color_tri_fragment_shader = color_tri_fragment_shader_p[index];
  color_tri_shader_program = color_tri_shader_program_p[index];
  color_tri_vertex_shader = color_tri_vertex_shader_p[index];

  texture_2D_fragment_shader = texture_2D_fragment_shader_p[index];
  texture_2D_shader_program = texture_2D_shader_program_p[index];
  texture_2D_vertex_shader = texture_2D_vertex_shader_p[index];

  fade_texture_2D_fragment_shader = fade_texture_2D_fragment_shader_p[index];
  fade_texture_2D_shader_program = fade_texture_2D_shader_program_p[index];
  fade_texture_2D_vertex_shader = fade_texture_2D_vertex_shader_p[index];

  circle_filled_shader_program = circle_filled_shader_program_p[index];
  circle_filled_vertex_shader = circle_filled_vertex_shader_p[index];
  circle_filled_fragment_shader = circle_filled_fragment_shader_p[index];

  FBO_texture_2D_fragment_shader = FBO_texture_2D_fragment_shader_p[index];
  FBO_texture_2D_shader_program = FBO_texture_2D_shader_program_p[index];
  FBO_texture_2D_vertex_shader = FBO_texture_2D_vertex_shader_p[index];

  texture_2DA_fragment_shader = texture_2DA_fragment_shader_p[index];
  texture_2DA_shader_program = texture_2DA_shader_program_p[index];
  texture_2DA_vertex_shader = texture_2DA_vertex_shader_p[index];
}

void unloadShaders() {
  color_tri_fragment_shader = color_tri_fragment_shader_p[0] =
      color_tri_fragment_shader_p[1] = 0;
  color_tri_shader_program = color_tri_shader_program_p[0] =
      color_tri_shader_program_p[1] = 0;
  color_tri_vertex_shader = color_tri_vertex_shader_p[0] =
      color_tri_vertex_shader_p[1] = 0;

  texture_2D_fragment_shader = texture_2D_fragment_shader_p[0] =
      texture_2D_fragment_shader_p[1];
  texture_2D_shader_program = texture_2D_shader_program_p[0] =
      texture_2D_shader_program_p[1] = 0;
  texture_2D_vertex_shader = texture_2D_vertex_shader_p[0] =
      texture_2D_vertex_shader_p[1] = 0;

  fade_texture_2D_fragment_shader = fade_texture_2D_fragment_shader_p[0] =
      fade_texture_2D_fragment_shader_p[1] = 0;
  fade_texture_2D_shader_program = fade_texture_2D_shader_program_p[0] =
      fade_texture_2D_shader_program_p[1] = 0;
  fade_texture_2D_vertex_shader = fade_texture_2D_vertex_shader_p[0] =
      fade_texture_2D_vertex_shader_p[1] = 0;

  circle_filled_shader_program = circle_filled_shader_program_p[0] =
      circle_filled_shader_program_p[1] = 0;
  circle_filled_vertex_shader = circle_filled_vertex_shader_p[0] =
      circle_filled_vertex_shader_p[1] = 0;
  circle_filled_fragment_shader = circle_filled_fragment_shader_p[0] =
      circle_filled_fragment_shader_p[1] = 0;

  FBO_texture_2D_fragment_shader = FBO_texture_2D_fragment_shader_p[0] =
      FBO_texture_2D_fragment_shader_p[1] = 0;
  FBO_texture_2D_shader_program = FBO_texture_2D_shader_program_p[0] =
      FBO_texture_2D_shader_program_p[1] = 0;
  FBO_texture_2D_vertex_shader = FBO_texture_2D_vertex_shader_p[0] =
      FBO_texture_2D_vertex_shader_p[1] = 0;

  texture_2DA_fragment_shader = texture_2DA_fragment_shader_p[0] =
      texture_2DA_fragment_shader_p[1] = 0;
  texture_2DA_shader_program = texture_2DA_shader_program_p[0] =
      texture_2DA_shader_program_p[1] = 0;
  texture_2DA_vertex_shader = texture_2DA_vertex_shader_p[0] =
      texture_2DA_vertex_shader_p[1] = 0;

  bShadersLoaded[0] = bShadersLoaded[1] = false;
}

#endif
