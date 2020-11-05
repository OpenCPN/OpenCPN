/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Radar Plugin
 * Author:   David Register
 *           Dave Cowell
 *           Kees Verruijt
 *           Douwe Fokkema
 *           Sean D'Epagnier
 *           Brian Paul
 ***************************************************************************
 *   Copyright (C) 2010 by David S. Register              bdbcat@yahoo.com *
 *   Copyright (C) 2012-2013 by Dave Cowell                                *
 *   Copyright (C) 2012-2016 by Kees Verruijt         canboat@verruijt.net *
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
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************
 */

#include "shaderutil.h"

#if defined(WIN32)
#define SET_FUNCTION_POINTER(name) wglGetProcAddress(name)
typedef PROC FunctionPointer;
#elif defined(__WXOSX__)
#include <dlfcn.h>
#define SET_FUNCTION_POINTER(name) dlsym(RTLD_DEFAULT, name)
typedef void *FunctionPointer;
#else
#include <GL/glx.h>
#define SET_FUNCTION_POINTER(name) glXGetProcAddress((const GLubyte *)name)
typedef __GLXextFuncPtr FunctionPointer;
#endif

#define SHADER_FUNCTION_LIST(proc, name) proc name;
#include "shaderutil.inc"
#undef SHADER_FUNCTION_LIST

PLUGIN_BEGIN_NAMESPACE

GLboolean ShadersSupported(void) {
  GLboolean ok = 1;

#define SHADER_FUNCTION_LIST(proc, name)    \
  {                                         \
    union {                                 \
      proc f;                               \
      FunctionPointer p;                    \
    } u;                                    \
    u.p = SET_FUNCTION_POINTER("gl" #name); \
    if (!u.p) ok = 0;                       \
    name = u.f;                             \
  }
#include "shaderutil.inc"
#undef SHADER_FUNCTION_LIST

  return ok;
}

bool CompileShaderText(GLuint *shader, GLenum shaderType, const char *text) {
  GLint stat;

  *shader = CreateShader(shaderType);
  ShaderSource(*shader, 1, (const GLchar **)&text, NULL);

  CompileShader(*shader);

  GetShaderiv(*shader, GL_COMPILE_STATUS, &stat);
  if (!stat) {
    GLchar log[1000];
    GLsizei len;
    GetShaderInfoLog(*shader, 1000, &len, log);
    wxLogError(wxT("radar_pi: problem compiling shader: %s"), log);
    return false;
  }
  return true;
}

GLuint LinkShaders(GLuint vertShader, GLuint fragShader) { return LinkShaders3(vertShader, 0, fragShader); }

GLuint LinkShaders3(GLuint vertShader, GLuint geomShader, GLuint fragShader) {
  GLuint program = CreateProgram();

  assert(vertShader || fragShader);

  if (vertShader) AttachShader(program, vertShader);
  if (geomShader) AttachShader(program, geomShader);
  if (fragShader) AttachShader(program, fragShader);

  LinkProgram(program);

  /* check link */
  {
    GLint stat;
    GetProgramiv(program, GL_LINK_STATUS, &stat);
    if (!stat) {
      GLchar log[1000];
      GLsizei len;
      GetProgramInfoLog(program, 1000, &len, log);
      wxLogError(wxT("radar_pi: problem linking program: %s"), log);
      return 0;
    }
  }

  return program;
}

GLboolean ValidateShaderProgram(GLuint program) {
  GLint stat;
  ValidateProgram(program);
  GetProgramiv(program, GL_VALIDATE_STATUS, &stat);

  if (!stat) {
    GLchar log[1000];
    GLsizei len;
    GetProgramInfoLog(program, 1000, &len, log);
    wxLogError(wxT("radar_pi: program validation error: %s"), log);
    return 0;
  }

  return (GLboolean)stat;
}

PLUGIN_END_NAMESPACE
