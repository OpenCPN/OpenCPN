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
const GLchar* preamble =
"\n";
#else
const GLchar* preamble =
"#version 120\n"
"#define precision\n"
"#define lowp\n"
"#define mediump\n"
"#define highp\n";
#endif

class GLShaderProgram
{
public:

    class Builder {
    public:
        Builder() : linked_(false) {
            programId_ = glCreateProgram();
        }

        Builder &addShaderFromSource(std::string const &shaderSource, GLenum shaderType) {
            char const *shaderCStr = shaderSource.c_str();
            GLuint shaderId = glCreateShader(shaderType);

            GLchar const* files[] = { preamble, shaderCStr };
            GLint lengths[]       = { (GLint)strlen(preamble),  (GLint)strlen(shaderCStr)  };

            glShaderSource(shaderId, 2, files, lengths);
            //glShaderSource(shaderId, 1, &shaderCStr, nullptr);

            glCompileShader(shaderId);
            glGetShaderiv(shaderId, GL_COMPILE_STATUS, &success);
            if (!success) {
              glGetShaderInfoLog(shaderId, 512, NULL, infoLog);
              printf("ERROR::SHADER::COMPILATION_FAILED\n%s\n", infoLog);
#ifdef USE_ANDROID_GLES2
              qDebug() << "SHADER COMPILE ERROR  " << infoLog;
#endif
              //ret_val = false;
            }

            glAttachShader(programId_, shaderId);
            return *this;
        }

        Builder &addShaderFromFile(std::string const &shaderFile, GLenum shaderType) {
            std::ifstream fileName(shaderFile);
            std::istreambuf_iterator<char> fileBegin(fileName), fileEnd;
            std::string fileContents(fileBegin, fileEnd);
            return addShaderFromSource(fileContents, shaderType);
        }

        GLShaderProgram linkProgram() {
            glLinkProgram(programId_);
            glGetProgramiv( programId_, GL_LINK_STATUS, &linkSuccess); //requesting the status
            if (linkSuccess == GL_FALSE) {
              glGetProgramInfoLog(programId_, 512, NULL, infoLog);
              printf("ERROR::SHADER::LINK_FAILED\n%s\n", infoLog);
            }

            GLShaderProgram theProgram(programId_);
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
        GLchar infoLog[512];
    };

    GLShaderProgram() : programId_(0) { }

    GLShaderProgram(GLShaderProgram &&other) {
        *this = std::move(other);
    }

    GLShaderProgram &operator=(GLShaderProgram &&other)
    {
        programId_ = other.programId_;
        other.programId_ = 0;

        if (other.programId_ != 0) {
            glDeleteProgram(other.programId_);
        }

        return *this;
    }

    ~GLShaderProgram() { }

    GLuint programId() const { return programId_; }

    GLShaderProgram(GLShaderProgram const &other) = delete;
    GLShaderProgram &operator=(GLShaderProgram const &other) = delete;
private:
    GLShaderProgram(GLuint programId) : programId_(programId) { }
    GLuint programId_;
};

/*
auto shaderProgram = GLShaderProgram::Builder()
    .addShaderFromFile("vertex.glsl", GL_VERTEX_SHADER)
    .addShaderFromFile("fragment.glsl", GL_FRAGMENT_SHADER)
    .linkProgram();

glUseProgram(shaderProgram.programId());
*/

#if 0
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
         glGetShaderInfoLog(shaderId, INFOLOG_LEN, NULL, infoLog);
         printf("ERROR::SHADER::COMPILATION_FAILED\n%s\n", infoLog);
#ifdef USE_ANDROID_GLES2
         qDebug() << "SHADER COMPILE ERROR  " << infoLog;
#endif
         return false;
      }

      glAttachShader(programId_, shaderId);
      return true;
    }

    bool linkProgram() {
      glLinkProgram(programId_);
      glGetProgramiv( programId_, GL_LINK_STATUS, &linkSuccess); //requesting the status
      if (linkSuccess == GL_FALSE) {
        glGetProgramInfoLog(programId_, INFOLOG_LEN, NULL, infoLog);
        printf("ERROR::SHADER::LINK_FAILED\n%s\n", infoLog);
        return false;
      }
      linked_ = true;
      return true;
    }


    GLuint programId() const { return programId_; }
    bool isOK() const { return linked_; }

private:
    //GLShaderProgram(GLuint programId) : programId_(programId) { }
    GLuint programId_;
    bool linked_;
    GLint success;
    GLint linkSuccess;
    GLchar infoLog[INFOLOG_LEN];

};

#endif










/*
struct ProgramStage
{
    enum Type
    {
        Vertex,
        Fragment,
        Geometry,
        // ... what have you ...
    };

    Type type;
    std::string filename; // This could also be a source code string instead if it suits you.
};

std::unique_ptr<GLShaderProgram> createGLShaderProgram(const std::vector<ProgramStage> & stages);


  std::vector<ProgramStage> shaderStages;
  ProgramStage psv;
  psv.type = GL_VERTEX_SHADER;
  psv.source = color_tri_vertex_shader_source;
  shaderStages.push_back(psv);

  ProgramStage psf;
  psf.type = GL_FRAGMENT_SHADER;
  psf.source = color_tri_vertex_shader_source;
  shaderStages.push_back(psf);
  auto shaderProgram = createGLShaderProgram(shaderStages);
*/

// const GLchar* preamble = R"(
// #version 140
// #define lowp
// #define mediump
// #define highp
// )";



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


//https://vitaliburkov.wordpress.com/2016/09/17/simple-and-fast-high-quality-antialiased-lines-with-opengl/
static const GLchar* AALine_vertex_shader_source =
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


static const GLchar* AALine_fragment_shader_source =
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
    "    else\n"
    "      col.w *= pow(float((w-d)/w), uBlendFactor);\n"
    "    gl_FragColor = col;\n"
    "}\n";




//GLint color_tri_shader_program;
//GLint texture_2D_shader_program;
//GLint circle_filled_shader_program;
//GLint texture_2DA_shader_program;
GLint fade_texture_2D_shader_program;
GLint FBO_texture_2D_shader_program;

// Protos
//GLint color_tri_shader_program_p[2];
//GLint texture_2D_shader_program_p[2];
//GLint circle_filled_shader_program_p[2];
//GLint AALine_shader_program_p[2];
//GLint texture_2DA_shader_program_p[2];
GLint fade_texture_2D_shader_program_p[2];
GLint FBO_texture_2D_shader_program_p[2];


GLShaderProgramA *pAALine_shader_program[2];
GLShaderProgramA *pcolor_tri_shader_program[2];
GLShaderProgramA *ptexture_2D_shader_program[2];
GLShaderProgramA *pfade_texture_2D_shader_program[2];
GLShaderProgramA *pcircle_filled_shader_program[2];
GLShaderProgramA *pFBO_texture_2D_shader_program[2];
GLShaderProgramA *ptexture_2DA_shader_program[2];


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
//   if (!color_tri_shader_program_p[index]) {
//     auto shaderProgram = GLShaderProgram::Builder()
//      .addShaderFromSource(color_tri_vertex_shader_source, GL_VERTEX_SHADER)
//      .addShaderFromSource(color_tri_fragment_shader_source, GL_FRAGMENT_SHADER)
//      .linkProgram();
//
//     color_tri_shader_program_p[index] = shaderProgram.programId();
//   }

  if (!pcolor_tri_shader_program[index]) {
    GLShaderProgramA *shaderProgram = new GLShaderProgramA;
    shaderProgram->addShaderFromSource(color_tri_vertex_shader_source, GL_VERTEX_SHADER);
    shaderProgram->addShaderFromSource(color_tri_fragment_shader_source, GL_FRAGMENT_SHADER);
    shaderProgram->linkProgram();

    if (shaderProgram->isOK())
      pcolor_tri_shader_program[index] = shaderProgram;
  }


  // Simple 2D texture shader
//   if (!texture_2D_shader_program_p[index]) {
//     auto shaderProgram = GLShaderProgram::Builder()
//      .addShaderFromSource(texture_2D_vertex_shader_source, GL_VERTEX_SHADER)
//      .addShaderFromSource(texture_2D_fragment_shader_source, GL_FRAGMENT_SHADER)
//      .linkProgram();
//
//     texture_2D_shader_program_p[index] = shaderProgram.programId();
//   }

  if (!ptexture_2D_shader_program[index]) {
    GLShaderProgramA *shaderProgram = new GLShaderProgramA;
    shaderProgram->addShaderFromSource(texture_2D_vertex_shader_source, GL_VERTEX_SHADER);
    shaderProgram->addShaderFromSource(texture_2D_fragment_shader_source, GL_FRAGMENT_SHADER);
    shaderProgram->linkProgram();

    if (shaderProgram->isOK())
      ptexture_2D_shader_program[index] = shaderProgram;
  }


  // Fade texture shader
  if (!fade_texture_2D_shader_program_p[index]) {
    auto shaderProgram = GLShaderProgram::Builder()
     .addShaderFromSource(fade_texture_2D_vertex_shader_source, GL_VERTEX_SHADER)
     .addShaderFromSource(fade_texture_2D_fragment_shader_source, GL_FRAGMENT_SHADER)
     .linkProgram();

    fade_texture_2D_shader_program_p[index] = shaderProgram.programId();
  }

  // Circle shader
//   if (!circle_filled_shader_program_p[index]) {
//     auto shaderProgram = GLShaderProgram::Builder()
//      .addShaderFromSource(circle_filled_vertex_shader_source, GL_VERTEX_SHADER)
//      .addShaderFromSource(circle_filled_fragment_shader_source, GL_FRAGMENT_SHADER)
//      .linkProgram();
//
//     circle_filled_shader_program_p[index] = shaderProgram.programId();
//   }

  if (!pcircle_filled_shader_program[index]) {
    GLShaderProgramA *shaderProgram = new GLShaderProgramA;
    shaderProgram->addShaderFromSource(circle_filled_vertex_shader_source, GL_VERTEX_SHADER);
    shaderProgram->addShaderFromSource(circle_filled_fragment_shader_source, GL_FRAGMENT_SHADER);
    shaderProgram->linkProgram();

    if (shaderProgram->isOK())
      pcircle_filled_shader_program[index] = shaderProgram;
  }



  // FBO 2D texture shader
  if (!FBO_texture_2D_shader_program_p[index]) {
    auto shaderProgram = GLShaderProgram::Builder()
     .addShaderFromSource(FBO_texture_2D_vertex_shader_source, GL_VERTEX_SHADER)
     .addShaderFromSource(FBO_texture_2D_fragment_shader_source, GL_FRAGMENT_SHADER)
     .linkProgram();

    FBO_texture_2D_shader_program_p[index] = shaderProgram.programId();
  }
#if 0
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
#endif
  // 2D Alpha color texture shader
//   if (!texture_2DA_shader_program_p[index]) {
//     auto shaderProgram = GLShaderProgram::Builder()
//      .addShaderFromSource(texture_2DA_vertex_shader_source, GL_VERTEX_SHADER)
//      .addShaderFromSource(texture_2DA_fragment_shader_source, GL_FRAGMENT_SHADER)
//      .linkProgram();
//
//     texture_2DA_shader_program_p[index] = shaderProgram.programId();
//   }

  if (!ptexture_2DA_shader_program[index]) {
    GLShaderProgramA *shaderProgram = new GLShaderProgramA;
    shaderProgram->addShaderFromSource(texture_2DA_vertex_shader_source, GL_VERTEX_SHADER);
    shaderProgram->addShaderFromSource(texture_2DA_fragment_shader_source, GL_FRAGMENT_SHADER);
    shaderProgram->linkProgram();

    if (shaderProgram->isOK())
      ptexture_2DA_shader_program[index] = shaderProgram;
  }


  if (!pAALine_shader_program[index]) {
    GLShaderProgramA *shaderProgram = new GLShaderProgramA;
    shaderProgram->addShaderFromSource(AALine_fragment_shader_source, GL_FRAGMENT_SHADER);
    shaderProgram->addShaderFromSource(AALine_vertex_shader_source, GL_VERTEX_SHADER);
    shaderProgram->linkProgram();

    if (shaderProgram->isOK())
      pAALine_shader_program[index] = shaderProgram;
  }



  bShadersLoaded[index] = true;
  reConfigureShaders(index);

  return ret_val;
}

void reConfigureShaders(int index) {
  //color_tri_shader_program = pcolor_tri_shader_program[index]->programId();

  //texture_2D_shader_program = ptexture_2D_shader_program[index]->programId();

  //circle_filled_shader_program = pcircle_filled_shader_program[index]->programId();

  //texture_2DA_shader_program = texture_2DA_shader_program_p[index];

  fade_texture_2D_shader_program = fade_texture_2D_shader_program_p[index];

  FBO_texture_2D_shader_program = FBO_texture_2D_shader_program_p[index];


}

void unloadShaders() {
  //color_tri_fragment_shader = color_tri_fragment_shader_p[0] =
  //    color_tri_fragment_shader_p[1] = 0;
  //color_tri_shader_program = color_tri_shader_program_p[0] =
    //  color_tri_shader_program_p[1] = 0;
  //color_tri_vertex_shader = color_tri_vertex_shader_p[0] =
  //    color_tri_vertex_shader_p[1] = 0;

//  texture_2D_fragment_shader = texture_2D_fragment_shader_p[0] =
//      texture_2D_fragment_shader_p[1];
//   texture_2D_shader_program = texture_2D_shader_program_p[0] =
//       texture_2D_shader_program_p[1] = 0;
//  texture_2D_vertex_shader = texture_2D_vertex_shader_p[0] =
//      texture_2D_vertex_shader_p[1] = 0;

//  fade_texture_2D_fragment_shader = fade_texture_2D_fragment_shader_p[0] =
//      fade_texture_2D_fragment_shader_p[1] = 0;

//   fade_texture_2D_shader_program = fade_texture_2D_shader_program_p[0] =
//       fade_texture_2D_shader_program_p[1] = 0;

//   FBO_texture_2D_shader_program = FBO_texture_2D_shader_program_p[0] =
//       FBO_texture_2D_shader_program_p[1] = 0;
//
//   texture_2DA_shader_program = texture_2DA_shader_program_p[0] =
//       texture_2DA_shader_program_p[1] = 0;

  bShadersLoaded[0] = bShadersLoaded[1] = false;
}

#endif
