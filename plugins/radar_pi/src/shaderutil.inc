/******************************************************************************
 *
 * Project:  OpenCPN
 * Purpose:  Radar Plugin
 * Author:   David Register
 *           Dave Cowell
 *           Kees Verruijt
 *           Douwe Fokkema
 *           Sean D'Epagnier
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

/*
 * This file is included multiple times to work with defining externally
 * loaded functions from a shared library.
 */

SHADER_FUNCTION_LIST(PFNGLCREATESHADERPROC, CreateShader)
SHADER_FUNCTION_LIST(PFNGLDELETESHADERPROC, DeleteShader)
SHADER_FUNCTION_LIST(PFNGLSHADERSOURCEPROC, ShaderSource)
SHADER_FUNCTION_LIST(PFNGLGETSHADERIVPROC, GetShaderiv)
SHADER_FUNCTION_LIST(PFNGLGETSHADERINFOLOGPROC, GetShaderInfoLog)
SHADER_FUNCTION_LIST(PFNGLCREATEPROGRAMPROC, CreateProgram)
SHADER_FUNCTION_LIST(PFNGLDELETEPROGRAMPROC, DeleteProgram)
SHADER_FUNCTION_LIST(PFNGLATTACHSHADERPROC, AttachShader)
SHADER_FUNCTION_LIST(PFNGLLINKPROGRAMPROC, LinkProgram)
SHADER_FUNCTION_LIST(PFNGLUSEPROGRAMPROC, UseProgram)
SHADER_FUNCTION_LIST(PFNGLGETPROGRAMIVPROC, GetProgramiv)
SHADER_FUNCTION_LIST(PFNGLGETPROGRAMINFOLOGPROC, GetProgramInfoLog)
SHADER_FUNCTION_LIST(PFNGLVALIDATEPROGRAMPROC, ValidateProgram)
SHADER_FUNCTION_LIST(PFNGLUNIFORM1IPROC, Uniform1i)
SHADER_FUNCTION_LIST(PFNGLUNIFORM1FVPROC, Uniform1fv)
SHADER_FUNCTION_LIST(PFNGLUNIFORM2FVPROC, Uniform2fv)
SHADER_FUNCTION_LIST(PFNGLUNIFORM3FVPROC, Uniform3fv)
SHADER_FUNCTION_LIST(PFNGLUNIFORM4FVPROC, Uniform4fv)
SHADER_FUNCTION_LIST(PFNGLUNIFORMMATRIX4FVPROC, UniformMatrix4fv)
SHADER_FUNCTION_LIST(PFNGLGETACTIVEATTRIBPROC, GetActiveAttrib)
SHADER_FUNCTION_LIST(PFNGLGETATTRIBLOCATIONPROC, GetAttribLocation)
SHADER_FUNCTION_LIST(PFNGLGETUNIFORMLOCATIONPROC, GetUniformLocation)
SHADER_FUNCTION_LIST(PFNGLGETACTIVEUNIFORMPROC, GetActiveUniform)
SHADER_FUNCTION_LIST(PFNGLCOMPILESHADERPROC, CompileShader)
