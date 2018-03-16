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

#include "../glshim/include/GLES/gl2.h"

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
    "   gl_FragColor = ((texTwoColor * (1.0 - texAlpha)) + (texColor * texAlpha));\n"
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
    
    
bool loadShaders()
{
   
    bool ret_val = true;
    GLint success;

    enum Consts {INFOLOG_LEN = 512};
    GLchar infoLog[INFOLOG_LEN];
    
    // Are the shaders ready?
    
    // Simple colored triangle shader
        
    if(!color_tri_vertex_shader){
       /* Vertex shader */
      color_tri_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
      glShaderSource(color_tri_vertex_shader, 1, &color_tri_vertex_shader_source, NULL);
      glCompileShader(color_tri_vertex_shader);
      glGetShaderiv(color_tri_vertex_shader, GL_COMPILE_STATUS, &success);
      if (!success) {
        glGetShaderInfoLog(color_tri_vertex_shader, INFOLOG_LEN, NULL, infoLog);
        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
        ret_val = false;
      }
    }
    
    if(!color_tri_fragment_shader){
        /* Fragment shader */
      color_tri_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
      glShaderSource(color_tri_fragment_shader, 1, &color_tri_fragment_shader_source, NULL);
      glCompileShader(color_tri_fragment_shader);
      glGetShaderiv(color_tri_fragment_shader, GL_COMPILE_STATUS, &success);
      if (!success) {
        glGetShaderInfoLog(color_tri_fragment_shader, INFOLOG_LEN, NULL, infoLog);
        printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
        ret_val = false;
     }
    }

    if(!color_tri_shader_program){
      /* Link shaders */
      color_tri_shader_program = glCreateProgram();
      glAttachShader(color_tri_shader_program, color_tri_fragment_shader);
      glAttachShader(color_tri_shader_program, color_tri_vertex_shader);
      glLinkProgram(color_tri_shader_program);
      glGetProgramiv(color_tri_shader_program, GL_LINK_STATUS, &success);
      if (!success) {
        glGetProgramInfoLog(color_tri_shader_program, INFOLOG_LEN, NULL, infoLog);
        printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
        ret_val = false;
      }
    }


        // Simple 2D texture shader
        
    if(!texture_2D_vertex_shader){
       /* Vertex shader */
      texture_2D_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
      glShaderSource(texture_2D_vertex_shader, 1, &texture_2D_vertex_shader_source, NULL);
      glCompileShader(texture_2D_vertex_shader);
      glGetShaderiv(texture_2D_vertex_shader, GL_COMPILE_STATUS, &success);
      if (!success) {
        glGetShaderInfoLog(texture_2D_vertex_shader, INFOLOG_LEN, NULL, infoLog);
        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
        ret_val = false;
      }
    }
    
    if(!texture_2D_fragment_shader){
        /* Fragment shader */
      texture_2D_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
      glShaderSource(texture_2D_fragment_shader, 1, &texture_2D_fragment_shader_source, NULL);
      glCompileShader(texture_2D_fragment_shader);
      glGetShaderiv(texture_2D_fragment_shader, GL_COMPILE_STATUS, &success);
      if (!success) {
        glGetShaderInfoLog(texture_2D_fragment_shader, INFOLOG_LEN, NULL, infoLog);
        printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
        ret_val = false;
      }
    }

    if(!texture_2D_shader_program){
      /* Link shaders */
      texture_2D_shader_program = glCreateProgram();
      glAttachShader(texture_2D_shader_program, texture_2D_vertex_shader);
      glAttachShader(texture_2D_shader_program, texture_2D_fragment_shader);
      glLinkProgram(texture_2D_shader_program);
      glGetProgramiv(texture_2D_shader_program, GL_LINK_STATUS, &success);
      if (!success) {
        glGetProgramInfoLog(texture_2D_shader_program, INFOLOG_LEN, NULL, infoLog);
        printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
        ret_val = false;
      }
    }

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
    

    // Circle shader
    if(!circle_filled_vertex_shader){
        /* Vertex shader */
        circle_filled_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(circle_filled_vertex_shader, 1, &circle_filled_vertex_shader_source, NULL);
        glCompileShader(circle_filled_vertex_shader);
        glGetShaderiv(circle_filled_vertex_shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(circle_filled_vertex_shader, INFOLOG_LEN, NULL, infoLog);
            printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
            qDebug() << infoLog;
            ret_val = false;
        }
    }
    
    if(!circle_filled_fragment_shader){
        /* Fragment shader */
        circle_filled_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(circle_filled_fragment_shader, 1, &circle_filled_fragment_shader_source, NULL);
        glCompileShader(circle_filled_fragment_shader);
        glGetShaderiv(circle_filled_fragment_shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(circle_filled_fragment_shader, INFOLOG_LEN, NULL, infoLog);
            printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
            qDebug() << infoLog;
            ret_val = false;
        }
    }
    
    if(!circle_filled_shader_program){
        /* Link shaders */
        circle_filled_shader_program = glCreateProgram();
        glAttachShader(circle_filled_shader_program, circle_filled_vertex_shader);
        glAttachShader(circle_filled_shader_program, circle_filled_fragment_shader);
        glLinkProgram(circle_filled_shader_program);
        glGetProgramiv(circle_filled_shader_program, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(circle_filled_shader_program, INFOLOG_LEN, NULL, infoLog);
            printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
            qDebug() << infoLog;
            ret_val = false;
        }
    }
    

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

    // 2D Alpha color texture shader
    
    if(!texture_2DA_vertex_shader){
        /* Vertex shader */
        texture_2DA_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
        glShaderSource(texture_2DA_vertex_shader, 1, &texture_2DA_vertex_shader_source, NULL);
        glCompileShader(texture_2DA_vertex_shader);
        glGetShaderiv(texture_2DA_vertex_shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(texture_2DA_vertex_shader, INFOLOG_LEN, NULL, infoLog);
            printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n%s\n", infoLog);
            qDebug() << infoLog;
            ret_val = false;
        }
    }
    
    if(!texture_2DA_fragment_shader){
        /* Fragment shader */
        texture_2DA_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
        glShaderSource(texture_2DA_fragment_shader, 1, &texture_2DA_fragment_shader_source, NULL);
        glCompileShader(texture_2DA_fragment_shader);
        glGetShaderiv(texture_2DA_fragment_shader, GL_COMPILE_STATUS, &success);
        if (!success) {
            glGetShaderInfoLog(texture_2DA_fragment_shader, INFOLOG_LEN, NULL, infoLog);
            printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n%s\n", infoLog);
            qDebug() << infoLog;
            ret_val = false;
        }
    }
    
    if(!texture_2DA_shader_program){
        /* Link shaders */
        texture_2DA_shader_program = glCreateProgram();
        glAttachShader(texture_2DA_shader_program, texture_2DA_vertex_shader);
        glAttachShader(texture_2DA_shader_program, texture_2DA_fragment_shader);
        glLinkProgram(texture_2DA_shader_program);
        glGetProgramiv(texture_2DA_shader_program, GL_LINK_STATUS, &success);
        if (!success) {
            glGetProgramInfoLog(texture_2DA_shader_program, INFOLOG_LEN, NULL, infoLog);
            printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n%s\n", infoLog);
            qDebug() << infoLog;
            ret_val = false;
        }
    }
    
    //qDebug() << "Shader Load " << ret_val;
    
    
    return ret_val;
}

#endif
