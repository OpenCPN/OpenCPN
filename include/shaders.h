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

#ifndef __SHADERS_H__
#define __SHADERS_H__

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // precompiled headers

#include "dychart.h"

#include <memory>
#include <vector>
#include <fstream>

class GLShaderProgramA;

//extern GLint color_tri_shader_program;
//extern GLint texture_2D_shader_program;
//extern GLint circle_filled_shader_program;
//extern GLint texture_2DA_shader_program;
extern GLint fade_texture_2D_shader_program;
extern GLint FBO_texture_2D_shader_program;

extern GLShaderProgramA *pAALine_shader_program[2];
extern GLShaderProgramA *pcolor_tri_shader_program[2];
extern GLShaderProgramA *ptexture_2D_shader_program[2];
extern GLShaderProgramA *pfade_texture_2D_shader_program[2];
extern GLShaderProgramA *pcircle_filled_shader_program[2];
extern GLShaderProgramA *pFBO_texture_2D_shader_program[2];
extern GLShaderProgramA *ptexture_2DA_shader_program[2];

extern const GLchar* preamble;

class GLShaderProgramA
{
public:
    GLShaderProgramA() : programId_(0), linked_(false) {
      programId_ = glCreateProgram();
    }
    ~GLShaderProgramA() { }

    bool addShaderFromSource(std::string const &shaderSource, GLenum shaderType) {
      char const *shaderCStr = shaderSource.c_str();
      GLuint shaderId = glCreateShader(shaderType);

      GLchar const* files[] = { preamble, shaderCStr };
      GLint lengths[]       = { (GLint)strlen(preamble),  (GLint)strlen(shaderCStr)  };

      glShaderSource(shaderId, 2, files, lengths);

      glCompileShader(shaderId);
      glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
      if (!success) {
        GLint logLength = 0;
        glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 0) {
          auto log = std::unique_ptr<char>(new char[logLength]);
          glGetShaderInfoLog(shaderId, logLength, &logLength, log.get());
          printf("ERROR::SHADER::COMPILATION_FAILED\n%s\n", log.get());
#ifdef USE_ANDROID_GLES2
          qDebug() << "SHADER COMPILE ERROR  " << log.get();
#endif
        }
         return false;
      }

      glAttachShader(programId_, shaderId);
      return true;
    }

    bool linkProgram() {
      glLinkProgram(programId_);
      glGetProgramiv( programId_, GL_LINK_STATUS, &linkSuccess); //requesting the status
      if (linkSuccess == GL_FALSE) {
        GLint logLength = 0;
        glGetShaderiv(programId_, GL_INFO_LOG_LENGTH, &logLength);
        if (logLength > 0) {
          auto log = std::unique_ptr<char>(new char[logLength]);
          glGetShaderInfoLog(programId_, logLength, &logLength, log.get());
          printf("ERROR::SHADER::LINK_FAILED\n%s\n", log.get());
        }
        return false;
      }
      linked_ = true;
      return true;
    }


    void Bind() { glUseProgram(programId_); }
    void UnBind() { glUseProgram(0); }

    void SetUniform1f( const char *name, float value) {
        GLint loc = glGetUniformLocation(programId_, name);
        glUniform1f( loc, value);
    }
    void SetUniform2fv( const char *name, float *value) {
        GLint loc = glGetUniformLocation(programId_, name);
        glUniform2fv( loc, 1, value);
    }
    void SetUniform4fv( const char *name, float *value) {
        GLint loc = glGetUniformLocation(programId_, name);
        glUniform4fv( loc, 1, value);
    }
    void SetUniform1i( const char *name, GLint value) {
        GLint loc = glGetUniformLocation(programId_, name);
        glUniform1i( loc, value);
    }

    void SetUniformMatrix4fv( const char *name, float *value) {
      GLint matloc = glGetUniformLocation(programId_, name);
      glUniformMatrix4fv(matloc, 1, GL_FALSE, value);
    }

    void SetAttributePointerf( const char *name, float *value ){
      GLint aloc = glGetAttribLocation(programId_, name);
      glVertexAttribPointer(aloc, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), value);
      glEnableVertexAttribArray(aloc);

      // Disable VBO's (vertex buffer objects) for attributes.
      glBindBuffer(GL_ARRAY_BUFFER, 0);
      glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    }




    GLuint programId() const { return programId_; }
    bool isOK() const { return linked_; }

private:
    //GLShaderProgram(GLuint programId) : programId_(programId) { }
    GLuint programId_;
    bool linked_;
    GLint success;
    GLint linkSuccess;

};


bool loadShaders(int index = 0);
void reConfigureShaders(int index = 0);
void unloadShaders();

#endif
