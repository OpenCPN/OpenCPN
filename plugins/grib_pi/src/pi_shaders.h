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

#ifndef __PISHADERS_H__
#define __PISHADERS_H__

#include "wx/wxprec.h"
#ifndef WX_PRECOMP
#include "wx/wx.h"
#endif  // precompiled headers

#include "pi_gl.h"

extern GLint GRIBpi_color_tri_shader_program;
extern GLint GRIBpi_colorv_tri_shader_program;
extern GLint pi_texture_2D_shader_program;
extern GLint pi_circle_filled_shader_program;

bool pi_loadShaders();
void configureShaders(float width, float height);

extern const GLchar* PI_shader_preamble;

enum Consts { INFOLOG_LEN = 512 };

class PI_GLShaderProgram
{
public:

    class Builder {
    public:
        Builder() : linked_(false) {
            programId_ = glCreateProgram();
        }

        Builder & addShaderFromSource(std::string const &shaderSource, GLenum shaderType) {
            char const *shaderCStr = shaderSource.c_str();
            GLuint shaderId = glCreateShader(shaderType);

            GLchar const* files[] = { PI_shader_preamble, shaderCStr };
            GLint lengths[]       = { (GLint)strlen(PI_shader_preamble),  (GLint)strlen(shaderCStr)  };

            glShaderSource(shaderId, 2, files, lengths);
            //glShaderSource(shaderId, 1, &shaderCStr, nullptr);

            glCompileShader(shaderId);
            glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
            if (!success) {
              glGetShaderInfoLog(shaderId, INFOLOG_LEN, NULL, infoLog);
              printf("ERROR::SHADER::COMPILATION_FAILED\n%s\n", infoLog);
              //ret_val = false;
            }

            glAttachShader(programId_, shaderId);
            return *this;
        }

//         void addShaderFromFile(std::string const &shaderFile, GLenum shaderType) {
//             std::ifstream fileName(shaderFile);
//             std::istreambuf_iterator<char> fileBegin(fileName), fileEnd;
//             std::string fileContents(fileBegin, fileEnd);
//             return addShaderFromSource(fileContents, shaderType);
//         }

        PI_GLShaderProgram linkProgram() {
            glLinkProgram(programId_);
            glGetProgramiv( programId_, GL_LINK_STATUS, &linkSuccess); //requesting the status
            if (linkSuccess == GL_FALSE) {
              glGetProgramInfoLog(programId_, INFOLOG_LEN, NULL, infoLog);
              printf("ERROR::SHADER::LINK_FAILED\n%s\n", infoLog);
            }

            PI_GLShaderProgram theProgram(programId_);
            //deleteAttachedShaders(programId_);
            linked_ = true;
            return theProgram;
        }

        ~Builder() {
             if(!linked_) {
                 glDeleteProgram(programId_);
             }
         }
    private:
        GLuint programId_;
        bool linked_;
        GLint success;
        GLint linkSuccess;
        GLchar infoLog[INFOLOG_LEN];
    };

    PI_GLShaderProgram() : programId_(0) { }

    PI_GLShaderProgram(PI_GLShaderProgram &&other) {
        *this = std::move(other);
    }

    PI_GLShaderProgram &operator=(PI_GLShaderProgram &&other)
    {
        programId_ = other.programId_;
        other.programId_ = 0;

        if (other.programId_ != 0) {
            glDeleteProgram(other.programId_);
        }

        return *this;
    }

    ~PI_GLShaderProgram() { }

    GLuint programId() const { return programId_; }

    PI_GLShaderProgram(PI_GLShaderProgram const &other) = delete;
    PI_GLShaderProgram &operator=(PI_GLShaderProgram const &other) = delete;
private:
    PI_GLShaderProgram(GLuint programId) : programId_(programId) { }
    GLuint programId_;
};


#endif
