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

#include "pi_shaders.h"

#include "linmath.h"

#ifdef USE_ANDROID_GLES2
#include "qdebug.h"
#include <GLES2/gl2.h>
#endif



#ifdef USE_ANDROID_GLES2
const GLchar* PI_shader_preamble =
"\n";
#else
const GLchar* PI_shader_preamble =
"#version 120\n"
"#define precision\n"
"#define lowp\n"
"#define mediump\n"
"#define highp\n";
#endif

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

//  Array colored triangle shader
static const GLchar* colorv_tri_vertex_shader_source =
    "attribute vec2 position;\n"
    "attribute vec4 colorv;\n"
    "uniform mat4 MVMatrix;\n"
    "uniform mat4 TransformMatrix;\n"
    "varying vec4 fragColor;\n"
    "void main() {\n"
    "   fragColor = colorv;\n"
    "   gl_Position = MVMatrix * TransformMatrix * vec4(position, 0.0, 1.0);\n"
    "}\n";

static const GLchar* colorv_tri_fragment_shader_source =
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

GLint pi_color_tri_fragment_shader;
GLint pi_color_tri_shader_program;
GLint pi_color_tri_vertex_shader;

GLint pi_colorv_tri_fragment_shader;
GLint pi_colorv_tri_shader_program;
GLint pi_colorv_tri_vertex_shader;

GLint pi_texture_2D_fragment_shader;
GLint pi_texture_2D_shader_program;
GLint pi_texture_2D_vertex_shader;

//     GLint fade_texture_2D_fragment_shader;
//     GLint fade_texture_2D_shader_program;
//     GLint fade_texture_2D_vertex_shader;

GLint pi_circle_filled_shader_program;
GLint pi_circle_filled_vertex_shader;
GLint pi_circle_filled_fragment_shader;

//     GLint FBO_texture_2D_fragment_shader;
//     GLint FBO_texture_2D_shader_program;
//     GLint FBO_texture_2D_vertex_shader;

bool pi_loadShaders() {
  bool ret_val = true;
  GLint success;

  enum Consts { INFOLOG_LEN = 512 };
  GLchar infoLog[INFOLOG_LEN];

  // Are the shaders ready?

  // Simple colored triangle shader

  if (!pi_color_tri_shader_program) {
    auto shaderProgram = PI_GLShaderProgram::Builder()
     .addShaderFromSource(color_tri_vertex_shader_source, GL_VERTEX_SHADER)
     .addShaderFromSource(color_tri_fragment_shader_source, GL_FRAGMENT_SHADER)
     .linkProgram();

    pi_color_tri_shader_program = shaderProgram.programId();
  }

#if 0
  if (!pi_color_tri_vertex_shader) {
    /* Vertex shader */
    pi_color_tri_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(pi_color_tri_vertex_shader, 1,
                   &color_tri_vertex_shader_source, NULL);
    glCompileShader(pi_color_tri_vertex_shader);
    glGetShaderiv(pi_color_tri_vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(pi_color_tri_vertex_shader, INFOLOG_LEN, NULL,
                         infoLog);
      printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
      ret_val = false;
    }
  }

  if (!pi_color_tri_fragment_shader) {
    /* Fragment shader */
    pi_color_tri_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(pi_color_tri_fragment_shader, 1,
                   &color_tri_fragment_shader_source, NULL);
    glCompileShader(pi_color_tri_fragment_shader);
    glGetShaderiv(pi_color_tri_fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(pi_color_tri_fragment_shader, INFOLOG_LEN, NULL,
                         infoLog);
      printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
      ret_val = false;
    }
  }

  if (!pi_color_tri_shader_program) {
    /* Link shaders */
    pi_color_tri_shader_program = glCreateProgram();
    glAttachShader(pi_color_tri_shader_program, pi_color_tri_fragment_shader);
    glAttachShader(pi_color_tri_shader_program, pi_color_tri_vertex_shader);
    glLinkProgram(pi_color_tri_shader_program);
    glGetProgramiv(pi_color_tri_shader_program, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(pi_color_tri_shader_program, INFOLOG_LEN, NULL,
                          infoLog);
      printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
      ret_val = false;
    }
  }
#endif

  // Array colored triangle shader
  if (!pi_colorv_tri_shader_program) {
    auto shaderProgram = PI_GLShaderProgram::Builder()
     .addShaderFromSource(colorv_tri_vertex_shader_source, GL_VERTEX_SHADER)
     .addShaderFromSource(colorv_tri_fragment_shader_source, GL_FRAGMENT_SHADER)
     .linkProgram();

    pi_colorv_tri_shader_program = shaderProgram.programId();
  }

#if 0
  if (!pi_colorv_tri_vertex_shader) {
    /* Vertex shader */
    pi_colorv_tri_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(pi_colorv_tri_vertex_shader, 1,
                   &colorv_tri_vertex_shader_source, NULL);
    glCompileShader(pi_colorv_tri_vertex_shader);
    glGetShaderiv(pi_colorv_tri_vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(pi_colorv_tri_vertex_shader, INFOLOG_LEN, NULL,
                         infoLog);
      printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
      ret_val = false;
    }
  }

  if (!pi_colorv_tri_fragment_shader) {
    /* Fragment shader */
    pi_colorv_tri_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(pi_colorv_tri_fragment_shader, 1,
                   &colorv_tri_fragment_shader_source, NULL);
    glCompileShader(pi_colorv_tri_fragment_shader);
    glGetShaderiv(pi_colorv_tri_fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(pi_colorv_tri_fragment_shader, INFOLOG_LEN, NULL,
                         infoLog);
      printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
      ret_val = false;
    }
  }

  if (!pi_colorv_tri_shader_program) {
    /* Link shaders */
    pi_colorv_tri_shader_program = glCreateProgram();
    glAttachShader(pi_colorv_tri_shader_program, pi_colorv_tri_fragment_shader);
    glAttachShader(pi_colorv_tri_shader_program, pi_colorv_tri_vertex_shader);
    glLinkProgram(pi_colorv_tri_shader_program);
    glGetProgramiv(pi_colorv_tri_shader_program, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(pi_colorv_tri_shader_program, INFOLOG_LEN, NULL,
                          infoLog);
      printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
      ret_val = false;
    }
  }
#endif

  // Simple 2D texture shader
  if (!pi_texture_2D_shader_program) {
    auto shaderProgram = PI_GLShaderProgram::Builder()
     .addShaderFromSource(texture_2D_vertex_shader_source, GL_VERTEX_SHADER)
     .addShaderFromSource(texture_2D_fragment_shader_source, GL_FRAGMENT_SHADER)
     .linkProgram();

    pi_texture_2D_shader_program = shaderProgram.programId();
  }

#if 0
  if (!pi_texture_2D_vertex_shader) {
    /* Vertex shader */
    pi_texture_2D_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(pi_texture_2D_vertex_shader, 1,
                   &texture_2D_vertex_shader_source, NULL);
    glCompileShader(pi_texture_2D_vertex_shader);
    glGetShaderiv(pi_texture_2D_vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(pi_texture_2D_vertex_shader, INFOLOG_LEN, NULL,
                         infoLog);
      printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
      ret_val = false;
    }
  }

  if (!pi_texture_2D_fragment_shader) {
    /* Fragment shader */
    pi_texture_2D_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(pi_texture_2D_fragment_shader, 1,
                   &texture_2D_fragment_shader_source, NULL);
    glCompileShader(pi_texture_2D_fragment_shader);
    glGetShaderiv(pi_texture_2D_fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(pi_texture_2D_fragment_shader, INFOLOG_LEN, NULL,
                         infoLog);
      printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
      ret_val = false;
    }
  }

  if (!pi_texture_2D_shader_program) {
    /* Link shaders */
    pi_texture_2D_shader_program = glCreateProgram();
    glAttachShader(pi_texture_2D_shader_program, pi_texture_2D_vertex_shader);
    glAttachShader(pi_texture_2D_shader_program, pi_texture_2D_fragment_shader);
    glLinkProgram(pi_texture_2D_shader_program);
    glGetProgramiv(pi_texture_2D_shader_program, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(pi_texture_2D_shader_program, INFOLOG_LEN, NULL,
                          infoLog);
      printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
      ret_val = false;
    }
  }
#endif

#if 0

    // Fade texture shader
    if(!fade_texture_2D_vertex_shader){
        /* Vertex shader */
        fade_texture_2D_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(fade_texture_2D_vertex_shader, 1, &fade_texture_2D_vertex_shader_source, NULL);
        glCompileShader(fade_texture_2D_vertex_shader);
        glGetShaderiv(fade_texture_2D_vertex_shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(fade_texture_2D_vertex_shader, INFOLOG_LEN, NULL, infoLog);
            printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
            ret_val = false;
        }
    }

    if(!fade_texture_2D_fragment_shader){
        /* Fragment shader */
        fade_texture_2D_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(fade_texture_2D_fragment_shader, 1, &fade_texture_2D_fragment_shader_source, NULL);
        glCompileShader(fade_texture_2D_fragment_shader);
        glGetShaderiv(fade_texture_2D_fragment_shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(fade_texture_2D_fragment_shader, INFOLOG_LEN, NULL, infoLog);
            printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
            ret_val = false;
        }
    }

    if(!fade_texture_2D_shader_program){
        /* Link shaders */
        fade_texture_2D_shader_program = glCreateProgram();
        glAttachShader(fade_texture_2D_shader_program, fade_texture_2D_vertex_shader);
        glAttachShader(fade_texture_2D_shader_program, fade_texture_2D_fragment_shader);
        glLinkProgram(fade_texture_2D_shader_program);
        glGetProgramiv(fade_texture_2D_shader_program, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(fade_texture_2D_shader_program, INFOLOG_LEN, NULL, infoLog);
            printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
            ret_val = false;
        }
    }

#endif

  // Circle shader
  if (!pi_circle_filled_shader_program) {
    auto shaderProgram = PI_GLShaderProgram::Builder()
     .addShaderFromSource(circle_filled_vertex_shader_source, GL_VERTEX_SHADER)
     .addShaderFromSource(circle_filled_fragment_shader_source, GL_FRAGMENT_SHADER)
     .linkProgram();

    pi_circle_filled_shader_program = shaderProgram.programId();
  }

#if 0
  if (!pi_circle_filled_vertex_shader) {
    /* Vertex shader */
    pi_circle_filled_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(pi_circle_filled_vertex_shader, 1,
                   &circle_filled_vertex_shader_source, NULL);
    glCompileShader(pi_circle_filled_vertex_shader);
    glGetShaderiv(pi_circle_filled_vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(pi_circle_filled_vertex_shader, INFOLOG_LEN, NULL,
                         infoLog);
      printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
      //qDebug() << infoLog;
      ret_val = false;
    }
  }

  if (!pi_circle_filled_fragment_shader) {
    /* Fragment shader */
    pi_circle_filled_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(pi_circle_filled_fragment_shader, 1,
                   &circle_filled_fragment_shader_source, NULL);
    glCompileShader(pi_circle_filled_fragment_shader);
    glGetShaderiv(pi_circle_filled_fragment_shader, GL_COMPILE_STATUS,
                  &success);
    if (!success) {
      glGetShaderInfoLog(pi_circle_filled_fragment_shader, INFOLOG_LEN, NULL,
                         infoLog);
      printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
      //qDebug() << infoLog;
      ret_val = false;
    }
  }

  if (!pi_circle_filled_shader_program) {
    /* Link shaders */
    pi_circle_filled_shader_program = glCreateProgram();
    glAttachShader(pi_circle_filled_shader_program,
                   pi_circle_filled_vertex_shader);
    glAttachShader(pi_circle_filled_shader_program,
                   pi_circle_filled_fragment_shader);
    glLinkProgram(pi_circle_filled_shader_program);
    glGetProgramiv(pi_circle_filled_shader_program, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(pi_circle_filled_shader_program, INFOLOG_LEN, NULL,
                          infoLog);
      printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
      //qDebug() << infoLog;
      ret_val = false;
    }
  }
#endif

#if 0
    // FBO 2D texture shader

    if(!FBO_texture_2D_vertex_shader){
        /* Vertex shader */
        FBO_texture_2D_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(FBO_texture_2D_vertex_shader, 1, &FBO_texture_2D_vertex_shader_source, NULL);
        glCompileShader(FBO_texture_2D_vertex_shader);
        glGetShaderiv(FBO_texture_2D_vertex_shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(FBO_texture_2D_vertex_shader, INFOLOG_LEN, NULL, infoLog);
            printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
            ret_val = false;
        }
    }

    if(!FBO_texture_2D_fragment_shader){
        /* Fragment shader */
        FBO_texture_2D_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(FBO_texture_2D_fragment_shader, 1, &FBO_texture_2D_fragment_shader_source, NULL);
        glCompileShader(FBO_texture_2D_fragment_shader);
        glGetShaderiv(FBO_texture_2D_fragment_shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(FBO_texture_2D_fragment_shader, INFOLOG_LEN, NULL, infoLog);
            printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
            ret_val = false;
        }
    }

    if(!FBO_texture_2D_shader_program){
        /* Link shaders */
        FBO_texture_2D_shader_program = glCreateProgram();
        glAttachShader(FBO_texture_2D_shader_program, FBO_texture_2D_vertex_shader);
        glAttachShader(FBO_texture_2D_shader_program, FBO_texture_2D_fragment_shader);
        glLinkProgram(FBO_texture_2D_shader_program);
        glGetProgramiv(FBO_texture_2D_shader_program, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(FBO_texture_2D_shader_program, INFOLOG_LEN, NULL, infoLog);
            printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
            ret_val = false;
        }
    }
#endif

  // qDebug() << "pi_loadShaders: " << ret_val;
  return ret_val;
}

void configureShaders(float width, float height) {
  //  Set the shader viewport transform matrix
  float vp_transform[16];
  mat4x4 m;
  mat4x4_identity(m);
  mat4x4_scale_aniso((float(*)[4])vp_transform, m, 2.0 / width, -2.0 / height,
                     1.0);
  mat4x4_translate_in_place((float(*)[4])vp_transform, -width / 2, -height / 2,
                            0);

  mat4x4 I;
  mat4x4_identity(I);

  glUseProgram(pi_color_tri_shader_program);
  GLint matloc = glGetUniformLocation(pi_color_tri_shader_program, "MVMatrix");
  glUniformMatrix4fv(matloc, 1, GL_FALSE, (const GLfloat*)vp_transform);
  GLint transloc =
      glGetUniformLocation(pi_color_tri_shader_program, "TransformMatrix");
  glUniformMatrix4fv(transloc, 1, GL_FALSE, (const GLfloat*)I);

  // qDebug() << pi_color_tri_shader_program << transloc;

  glUseProgram(pi_circle_filled_shader_program);
  matloc = glGetUniformLocation(pi_circle_filled_shader_program, "MVMatrix");
  glUniformMatrix4fv(matloc, 1, GL_FALSE, (const GLfloat*)vp_transform);
  transloc =
      glGetUniformLocation(pi_circle_filled_shader_program, "TransformMatrix");
  glUniformMatrix4fv(transloc, 1, GL_FALSE, (const GLfloat*)I);

  // qDebug() << pi_circle_filled_shader_program << transloc;

  glUseProgram(pi_texture_2D_shader_program);
  matloc = glGetUniformLocation(pi_texture_2D_shader_program, "MVMatrix");
  glUniformMatrix4fv(matloc, 1, GL_FALSE, (const GLfloat*)vp_transform);
  transloc =
      glGetUniformLocation(pi_texture_2D_shader_program, "TransformMatrix");
  glUniformMatrix4fv(transloc, 1, GL_FALSE, (const GLfloat*)I);

  // qDebug() << pi_texture_2D_shader_program << transloc;

  glUseProgram(pi_colorv_tri_shader_program);
  matloc = glGetUniformLocation(pi_colorv_tri_shader_program, "MVMatrix");
  glUniformMatrix4fv(matloc, 1, GL_FALSE, (const GLfloat*)vp_transform);
  transloc =
      glGetUniformLocation(pi_colorv_tri_shader_program, "TransformMatrix");
  glUniformMatrix4fv(transloc, 1, GL_FALSE, (const GLfloat*)I);
}

