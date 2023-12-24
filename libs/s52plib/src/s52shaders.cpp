/***************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  S52 Presentation Library
 * Authors:   David Register
 *            Jesper Weissglas
 *
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register                               *
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


#if defined(USE_ANDROID_GLES2) || defined(ocpnUSE_GLSL)

#include "s52shaders.h"

#ifdef USE_ANDROID_GLES2
const GLchar* S52_preamble =
"\n";
#else
const GLchar* S52_preamble =
"#version 120\n"
"#define precision\n"
"#define lowp\n"
"#define mediump\n"
"#define highp\n";
#endif


// Simple colored triangle shader

static const GLchar *S52color_tri_vertex_shader_source =
    "attribute vec2 position;\n"
    "uniform mat4 MVMatrix;\n"
    "uniform mat4 TransformMatrix;\n"
    "uniform vec4 color;\n"
    "varying vec4 fragColor;\n"
    "void main() {\n"
    "   fragColor = color;\n"
    "   gl_Position = MVMatrix * TransformMatrix  * vec4(position, 0.0, 1.0);\n"
    "}\n";

static const GLchar *S52color_tri_fragment_shader_source =
    "precision highp float;\n"
    "varying vec4 fragColor;\n"
    "void main() {\n"
    "   gl_FragColor = fragColor;\n"
    "}\n";

// Simple 2D texture shader
static const GLchar *S52texture_2D_vertex_shader_source =
    "attribute vec2 position;\n"
    "attribute vec2 aUV;\n"
    "uniform mat4 MVMatrix;\n"
    "uniform mat4 TransformMatrix;\n"
    "varying vec2 varCoord;\n"
    "void main() {\n"
    "   gl_Position = MVMatrix * TransformMatrix * vec4(position, 0.0, 1.0);\n"
    "   //varCoord = aUV.st;\n"
    "   varCoord = aUV;\n"
    "}\n";

static const GLchar *S52texture_2D_fragment_shader_source =
    "precision highp float;\n"
    "uniform sampler2D uTex;\n"
    "varying vec2 varCoord;\n"
    "void main() {\n"
    "   gl_FragColor = texture2D(uTex, varCoord);\n"
    "}\n";

// 2D texture shader with color modulation, used for colored text
static const GLchar *S52texture_2D_ColorMod_vertex_shader_source =
    "precision highp float;\n"
    "attribute vec2 position;\n"
    "attribute vec2 aUV;\n"
    "uniform mat4 MVMatrix;\n"
    "uniform mat4 TransformMatrix;\n"
    "varying vec2 varCoord;\n"
    "void main() {\n"
    "   gl_Position = MVMatrix * TransformMatrix * vec4(position, 0.0, 1.0);\n"
    "   //varCoord = aUV.st;\n"
    "   varCoord = aUV;\n"
    "}\n";

static const GLchar *S52texture_2D_ColorMod_fragment_shader_source =
    "precision highp float;\n"
    "uniform sampler2D uTex;\n"
    "uniform vec4 color;\n"
    "varying vec2 varCoord;\n"
    "void main() {\n"
    "   vec4 col=texture2D(uTex, varCoord);\n"
    "   gl_FragColor = color;\n"
    "   gl_FragColor.a = col.a;\n"
    "}\n";

//  Circle shader

static const GLchar *S52circle_filled_vertex_shader_source =
    "precision highp float;\n"
    "attribute vec2 aPos;\n"
    "uniform mat4 MVMatrix;\n"
    "uniform mat4 TransformMatrix;\n"
    "void main() {\n"
    "   gl_Position = MVMatrix * TransformMatrix * vec4(aPos, 0.0, 1.0);\n"
    "}\n";

static const GLchar *S52circle_filled_fragment_shader_source =
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

//  Ring shader

static const GLchar *S52ring_vertex_shader_source =
    "precision highp float;\n"
    "attribute vec2 aPos;\n"
    "uniform mat4 MVMatrix;\n"
    "uniform mat4 TransformMatrix;\n"
    "void main() {\n"
    "   gl_Position = MVMatrix * TransformMatrix * vec4(aPos, 0.0, 1.0);\n"
    "}\n";

static const GLchar *S52ring_fragment_shader_source_old =
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


static const GLchar *S52ring_fragment_shader_source =
    "precision highp float;\n"
    "uniform float ring_width;\n"
    "uniform float circle_radius;\n"
    "uniform vec4 ring_color;\n"
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
    "   float delta = 0.9;\n"
    "   if (d > circle_radius + ring_width/2.) {\n"
    "       discard;\n"
    "   } else if( d > (circle_radius )) {\n"
    "     float alpha = smoothstep(circle_radius + ring_width/2. -delta, circle_radius + ring_width/2., d);\n"
    "     gl_FragColor = ring_color;\n"
    "     gl_FragColor.w = 1.-alpha;\n"
    "   }\n"
    "   else if( d > (circle_radius - ring_width/2. - delta)) {\n"
    "       gl_FragColor = ring_color;\n"
    "       float alpha = smoothstep(circle_radius - ring_width/2. - delta, circle_radius - ring_width/2., d);\n"
    "       gl_FragColor.w = alpha;\n"
    "   }\n"
    "   else  {\n"
    "       discard;\n"
    "   }\n"
    "} else{\n"
    "   discard;\n"
    "}\n"
    "}\n";

// Dash/Dot line shader
static const GLchar *S52Dash_vertex_shader_source =
    "attribute vec2 position;\n"
    "uniform mat4 MVMatrix;\n"
    "uniform mat4 TransformMatrix;\n"
    "void main() {\n"
    "   gl_Position = MVMatrix * TransformMatrix * vec4(position, 0.0, 1.0);\n"
    "}\n";

static const GLchar *S52Dash_fragment_shader_source =
    "precision highp float;\n"
    "uniform sampler2D uTex;\n"
    "uniform vec2 startPos;\n"
    "uniform float texWidth;\n"
    "uniform float dashFactor;\n"
    "uniform vec4 color;\n"
    "void main() {\n"
    "   float d = distance(gl_FragCoord.xy, startPos);\n"
    "   float x = mod(d,texWidth) / texWidth;\n"
    "   if(x < dashFactor) gl_FragColor = color;\n"
    "   else gl_FragColor = vec4(0.0, 0.0, 0.0, 0.0);\n"
    "}\n";

// Texture shader for Area Pattern
static const GLchar *S52AP_vertex_shader_source =
    "attribute vec2 position;\n"
    "attribute vec2 aUV;\n"
    "uniform mat4 MVMatrix;\n"
    "uniform mat4 TransformMatrix;\n"
    "void main() {\n"
    "   gl_Position = MVMatrix * TransformMatrix * vec4(position, 0.0, 1.0);\n"
    "}\n";

#if 0
static const GLchar* S52AP_fragment_shader_source =
    "precision highp float;\n"
    "uniform sampler2D uTex;\n"
    "uniform float texWidth;\n"
    "uniform float texHeight;\n"
    "uniform float texPOTWidth;\n"
    "uniform float texPOTHeight;\n"
    "uniform float staggerFactor;\n"
    "uniform vec4 color;\n"
    "uniform float xOff;\n"
    "uniform float yOff;\n"
    "void main() {\n"
    "   float yp = floor((gl_FragCoord.y + yOff) / texHeight);\n"
    "   float fstagger = 0.0;\n"
    "   //if(mod(yp, 2.0) < 0.1) fstagger = staggerFactor;\n"
    "   float xStag = xOff + (fstagger * texWidth);\n"
    "   float x = mod((gl_FragCoord.x - xStag),texWidth) / texPOTWidth;\n"
    "   float y = mod((gl_FragCoord.y + yOff),texHeight) / texPOTHeight;\n"
     "   gl_FragColor = texture2D(uTex, vec2(x, (texHeight / texPOTHeight) - y));\n"
    "}\n";
#else

static const GLchar *S52AP_fragment_shader_source =
    "precision highp float;\n"
    "uniform sampler2D uTex;\n"
    "uniform float texWidth;\n"
    "uniform float texHeight;\n"
    "uniform float texPOTWidth;\n"
    "uniform float texPOTHeight;\n"
    "uniform float staggerFactor;\n"
    "uniform vec4 color;\n"
    "uniform float xOff;\n"
    "uniform float yOff;\n"
    "uniform float yOffM;\n"
    "void main() {\n"
    "   float yp = floor((gl_FragCoord.y + yOffM ) / texPOTHeight);\n"
    "   float fstagger = 0.0;\n"
    "   if(mod(yp, 2.0) < 0.1) fstagger = 0.5;\n"
    "   float x = (gl_FragCoord.x - xOff) / texPOTWidth;\n"
    "   float y = (gl_FragCoord.y + yOff) / texPOTHeight;\n"
    "   gl_FragColor = texture2D(uTex, vec2(x + fstagger, -y));\n"
    "}\n";
#endif

GLint S52color_tri_fragment_shader;
GLint S52color_tri_vertex_shader;

GLint S52texture_2D_fragment_shader;
GLint S52texture_2D_vertex_shader;

GLint S52texture_2D_ColorMod_fragment_shader;
GLint S52texture_2D_ColorMod_vertex_shader;

GLint S52circle_filled_vertex_shader;
GLint S52circle_filled_fragment_shader;

GLint S52ring_vertex_shader;
GLint S52ring_fragment_shader;

GLint S52Dash_vertex_shader;
GLint S52Dash_fragment_shader;

GLint S52AP_vertex_shader;
GLint S52AP_fragment_shader;

extern GLint S52color_tri_shader_program;
extern GLint S52texture_2D_shader_program;
extern GLint S52texture_2D_ColorMod_shader_program;
extern GLint S52circle_filled_shader_program;
extern GLint S52ring_shader_program;
extern GLint S52Dash_shader_program;
extern GLint S52AP_shader_program;

S52_GLShaderProgram *pS52texture_2D_ColorMod_shader_program;


bool shadersLoaded = false;

bool loadS52Shaders() {
  bool ret_val = true;
  //GLint success;

  enum Consts { INFOLOG_LEN = 512 };
#if 0
  GLchar infoLog[INFOLOG_LEN];
#endif

  // Are the shaders ready?
  if(shadersLoaded)
    return true;

  // Simple colored triangle shader

  if (!S52color_tri_vertex_shader) {
    auto shaderProgram = S52_GLShaderProgram::Builder()
     .addShaderFromSource(S52color_tri_vertex_shader_source, GL_VERTEX_SHADER)
     .addShaderFromSource(S52color_tri_fragment_shader_source, GL_FRAGMENT_SHADER)
     .linkProgram();

    S52color_tri_shader_program = shaderProgram.programId();
  }


  if (!S52texture_2D_shader_program) {
    auto shaderProgram = S52_GLShaderProgram::Builder()
     .addShaderFromSource(S52texture_2D_vertex_shader_source, GL_VERTEX_SHADER)
     .addShaderFromSource(S52texture_2D_fragment_shader_source, GL_FRAGMENT_SHADER)
     .linkProgram();

    S52texture_2D_shader_program = shaderProgram.programId();
  }

#if 0
  // Simple 2D texture shader

  if (!S52texture_2D_vertex_shader) {
    /* Vertex shader */
    S52texture_2D_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(S52texture_2D_vertex_shader, 1,
                   &S52texture_2D_vertex_shader_source, NULL);
    glCompileShader(S52texture_2D_vertex_shader);
    glGetShaderiv(S52texture_2D_vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(S52texture_2D_vertex_shader, INFOLOG_LEN, NULL,
                         infoLog);
      //        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n",
      //        infoLog);
      ret_val = false;
    }
  }

  if (!S52texture_2D_fragment_shader) {
    /* Fragment shader */
    S52texture_2D_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(S52texture_2D_fragment_shader, 1,
                   &S52texture_2D_fragment_shader_source, NULL);
    glCompileShader(S52texture_2D_fragment_shader);
    glGetShaderiv(S52texture_2D_fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(S52texture_2D_fragment_shader, INFOLOG_LEN, NULL,
                         infoLog);
      //        printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n",
      //        infoLog);
      ret_val = false;
    }
  }

  if (!S52texture_2D_shader_program) {
    /* Link shaders */
    S52texture_2D_shader_program = glCreateProgram();
    glAttachShader(S52texture_2D_shader_program, S52texture_2D_vertex_shader);
    glAttachShader(S52texture_2D_shader_program, S52texture_2D_fragment_shader);
    glLinkProgram(S52texture_2D_shader_program);
    glGetProgramiv(S52texture_2D_shader_program, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(S52texture_2D_shader_program, INFOLOG_LEN, NULL,
                          infoLog);
      //        printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
      ret_val = false;
    }
  }
#endif

  // 2D texture shader with color Modulate
  if (!S52texture_2D_ColorMod_shader_program) {

    auto shaderProgram = S52_GLShaderProgram::Builder()
     .addShaderFromSource(S52texture_2D_ColorMod_vertex_shader_source, GL_VERTEX_SHADER)
     .addShaderFromSource(S52texture_2D_ColorMod_fragment_shader_source, GL_FRAGMENT_SHADER)
     .linkProgram();

    S52texture_2D_ColorMod_shader_program = shaderProgram.programId();

  }

#if 0
  if (!S52texture_2D_ColorMod_vertex_shader) {
    /* Vertex shader */
    S52texture_2D_ColorMod_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(S52texture_2D_ColorMod_vertex_shader, 1,
                   &S52texture_2D_ColorMod_vertex_shader_source, NULL);
    glCompileShader(S52texture_2D_ColorMod_vertex_shader);
    glGetShaderiv(S52texture_2D_ColorMod_vertex_shader, GL_COMPILE_STATUS,
                  &success);
    if (!success) {
      glGetShaderInfoLog(S52texture_2D_ColorMod_vertex_shader, INFOLOG_LEN,
                         NULL, infoLog);
      //        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n",
      //        infoLog);
      ret_val = false;
    }
  }


  if (!S52texture_2D_ColorMod_fragment_shader) {
    /* Fragment shader */
    S52texture_2D_ColorMod_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(S52texture_2D_ColorMod_fragment_shader, 1,
                   &S52texture_2D_ColorMod_fragment_shader_source, NULL);
    glCompileShader(S52texture_2D_ColorMod_fragment_shader);
    glGetShaderiv(S52texture_2D_ColorMod_fragment_shader, GL_COMPILE_STATUS,
                  &success);
    if (!success) {
      glGetShaderInfoLog(S52texture_2D_ColorMod_fragment_shader, INFOLOG_LEN,
                         NULL, infoLog);
      //        printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n",
      //        infoLog);
      ret_val = false;
    }
  }

  if (!S52texture_2D_ColorMod_shader_program) {
    /* Link shaders */
    S52texture_2D_ColorMod_shader_program = glCreateProgram();
    glAttachShader(S52texture_2D_ColorMod_shader_program,
                   S52texture_2D_ColorMod_vertex_shader);
    glAttachShader(S52texture_2D_ColorMod_shader_program,
                   S52texture_2D_ColorMod_fragment_shader);
    glLinkProgram(S52texture_2D_ColorMod_shader_program);
    glGetProgramiv(S52texture_2D_ColorMod_shader_program, GL_LINK_STATUS,
                   &success);
    if (!success) {
      glGetProgramInfoLog(S52texture_2D_ColorMod_shader_program, INFOLOG_LEN,
                          NULL, infoLog);
      //        printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
      ret_val = false;
    }
  }
#endif

  // Circle shader
  if (!S52circle_filled_shader_program) {
    auto shaderProgram = S52_GLShaderProgram::Builder()
     .addShaderFromSource(S52circle_filled_vertex_shader_source, GL_VERTEX_SHADER)
     .addShaderFromSource(S52circle_filled_fragment_shader_source, GL_FRAGMENT_SHADER)
     .linkProgram();

    S52circle_filled_shader_program = shaderProgram.programId();
  }

#if 0
  if (!S52circle_filled_vertex_shader) {
    /* Vertex shader */
    S52circle_filled_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(S52circle_filled_vertex_shader, 1,
                   &S52circle_filled_vertex_shader_source, NULL);
    glCompileShader(S52circle_filled_vertex_shader);
    glGetShaderiv(S52circle_filled_vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(S52circle_filled_vertex_shader, INFOLOG_LEN, NULL,
                         infoLog);
      //            printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n",
      //            infoLog);
      ret_val = false;
    }
  }

  if (!S52circle_filled_fragment_shader) {
    /* Fragment shader */
    S52circle_filled_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(S52circle_filled_fragment_shader, 1,
                   &S52circle_filled_fragment_shader_source, NULL);
    glCompileShader(S52circle_filled_fragment_shader);
    glGetShaderiv(S52circle_filled_fragment_shader, GL_COMPILE_STATUS,
                  &success);
    if (!success) {
      glGetShaderInfoLog(S52circle_filled_fragment_shader, INFOLOG_LEN, NULL,
                         infoLog);
      //            printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n",
      //            infoLog);
      ret_val = false;
    }
  }

  if (!S52circle_filled_shader_program) {
    /* Link shaders */
    S52circle_filled_shader_program = glCreateProgram();
    glAttachShader(S52circle_filled_shader_program,
                   S52circle_filled_vertex_shader);
    glAttachShader(S52circle_filled_shader_program,
                   S52circle_filled_fragment_shader);
    glLinkProgram(S52circle_filled_shader_program);
    glGetProgramiv(S52circle_filled_shader_program, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(S52circle_filled_shader_program, INFOLOG_LEN, NULL,
                          infoLog);
      //            printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n",
      //            infoLog);
      ret_val = false;
    }
  }
#endif

  // Ring shader
  if (!S52ring_shader_program) {
    auto shaderProgram = S52_GLShaderProgram::Builder()
     .addShaderFromSource(S52ring_vertex_shader_source, GL_VERTEX_SHADER)
     .addShaderFromSource(S52ring_fragment_shader_source, GL_FRAGMENT_SHADER)
     .linkProgram();

    S52ring_shader_program = shaderProgram.programId();
  }

#if 0
  if (!S52ring_vertex_shader) {
    /* Vertex shader */
    S52ring_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(S52ring_vertex_shader, 1, &S52ring_vertex_shader_source,
                   NULL);
    glCompileShader(S52ring_vertex_shader);
    glGetShaderiv(S52ring_vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(S52ring_vertex_shader, INFOLOG_LEN, NULL, infoLog);
      //           printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n",
      //           infoLog);
      ret_val = false;
    }
  }

  if (!S52ring_fragment_shader) {
    /* Fragment shader */
    S52ring_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(S52ring_fragment_shader, 1, &S52ring_fragment_shader_source,
                   NULL);
    glCompileShader(S52ring_fragment_shader);
    glGetShaderiv(S52ring_fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(S52ring_fragment_shader, INFOLOG_LEN, NULL, infoLog);
      //            printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n",
      //            infoLog);
      ret_val = false;
    }
  }

  if (!S52ring_shader_program) {
    /* Link shaders */
    S52ring_shader_program = glCreateProgram();
    glAttachShader(S52ring_shader_program, S52ring_vertex_shader);
    glAttachShader(S52ring_shader_program, S52ring_fragment_shader);
    glLinkProgram(S52ring_shader_program);
    glGetProgramiv(S52ring_shader_program, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(S52ring_shader_program, INFOLOG_LEN, NULL, infoLog);
      //            printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n",
      //            infoLog);
      ret_val = false;
    }
  }
#endif

  // Dash shader
  if (!S52Dash_shader_program) {
    auto shaderProgram = S52_GLShaderProgram::Builder()
     .addShaderFromSource(S52Dash_vertex_shader_source, GL_VERTEX_SHADER)
     .addShaderFromSource(S52Dash_fragment_shader_source, GL_FRAGMENT_SHADER)
     .linkProgram();

    S52Dash_shader_program = shaderProgram.programId();
  }

#if 0
  if (!S52Dash_vertex_shader) {
    /* Vertex shader */
    S52Dash_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(S52Dash_vertex_shader, 1, &S52Dash_vertex_shader_source,
                   NULL);
    glCompileShader(S52Dash_vertex_shader);
    glGetShaderiv(S52Dash_vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(S52ring_vertex_shader, INFOLOG_LEN, NULL, infoLog);
      //            printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n",
      //            infoLog);
      ret_val = false;
    }
  }

  if (!S52Dash_fragment_shader) {
    /* Fragment shader */
    S52Dash_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(S52Dash_fragment_shader, 1, &S52Dash_fragment_shader_source,
                   NULL);
    glCompileShader(S52Dash_fragment_shader);
    glGetShaderiv(S52Dash_fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(S52Dash_fragment_shader, INFOLOG_LEN, NULL, infoLog);
      //            printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n",
      //            infoLog);
      ret_val = false;
    }
  }

  if (!S52Dash_shader_program) {
    /* Link shaders */
    S52Dash_shader_program = glCreateProgram();
    glAttachShader(S52Dash_shader_program, S52Dash_vertex_shader);
    glAttachShader(S52Dash_shader_program, S52Dash_fragment_shader);
    glLinkProgram(S52Dash_shader_program);
    glGetProgramiv(S52Dash_shader_program, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(S52Dash_shader_program, INFOLOG_LEN, NULL, infoLog);
      //            printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n",
      //            infoLog);
      ret_val = false;
    }
  }
#endif

  // AP shader
    if (!S52AP_shader_program) {
    auto shaderProgram = S52_GLShaderProgram::Builder()
     .addShaderFromSource(S52AP_vertex_shader_source, GL_VERTEX_SHADER)
     .addShaderFromSource(S52AP_fragment_shader_source, GL_FRAGMENT_SHADER)
     .linkProgram();

    S52AP_shader_program = shaderProgram.programId();
  }

#if 0
  if (!S52AP_vertex_shader) {
    /* Vertex shader */
    S52AP_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(S52AP_vertex_shader, 1, &S52AP_vertex_shader_source, NULL);
    glCompileShader(S52AP_vertex_shader);
    glGetShaderiv(S52AP_vertex_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(S52ring_vertex_shader, INFOLOG_LEN, NULL, infoLog);
      //            printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n",
      //            infoLog);
      ret_val = false;
    }
  }

  if (!S52AP_fragment_shader) {
    /* Fragment shader */
    S52AP_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(S52AP_fragment_shader, 1, &S52AP_fragment_shader_source,
                   NULL);
    glCompileShader(S52AP_fragment_shader);
    glGetShaderiv(S52AP_fragment_shader, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(S52AP_fragment_shader, INFOLOG_LEN, NULL, infoLog);
      //          printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n",
      //          infoLog);
      ret_val = false;
    }
  }

  if (!S52AP_shader_program) {
    /* Link shaders */
    S52AP_shader_program = glCreateProgram();
    glAttachShader(S52AP_shader_program, S52AP_vertex_shader);
    glAttachShader(S52AP_shader_program, S52AP_fragment_shader);
    glLinkProgram(S52AP_shader_program);
    glGetProgramiv(S52AP_shader_program, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(S52AP_shader_program, INFOLOG_LEN, NULL, infoLog);
      //            printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n",
      //            infoLog);
      ret_val = false;
    }
  }
#endif
  shadersLoaded = true;

  return ret_val;
}


#endif
