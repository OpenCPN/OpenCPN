#ifndef _SHADER_UTIL_H_
#define _SHADER_UTIL_H_

#include "pi_common.h"

PLUGIN_BEGIN_NAMESPACE

struct uniform_info {
  const char *name;
  GLuint size; /**< number of value[] elements: 1, 2, 3 or 4 */
  GLenum type; /**< GL_FLOAT, GL_FLOAT_VEC4, GL_INT, GL_FLOAT_MAT4, etc */
  GLfloat value[16];
  GLint location; /**< filled in by InitUniforms() */
};

#define END_OF_UNIFORMS \
  { NULL, 0, GL_NONE, {0, 0, 0, 0}, -1 }

struct attrib_info {
  const char *name;
  GLuint size; /**< number of value[] elements: 1, 2, 3 or 4 */
  GLenum type; /**< GL_FLOAT, GL_FLOAT_VEC4, GL_INT, etc */
  GLint location;
};

extern GLboolean ShadersSupported(void);

extern bool CompileShaderText(GLuint *shader, GLenum shaderType, const char *text);

extern GLuint LinkShaders(GLuint vertShader, GLuint fragShader);

extern GLuint LinkShaders3(GLuint vertShader, GLuint geomShader, GLuint fragShader);

extern GLuint LinkShaders3WithGeometryInfo(GLuint vertShader, GLuint geomShader, GLuint fragShader, GLint verticesOut,
                                           GLenum inputType, GLenum outputType);

extern GLboolean ValidateShaderProgram(GLuint program);

extern GLdouble GetShaderCompileTime(void);

extern GLdouble GetShaderLinkTime(void);

extern void SetUniformValues(GLuint program, struct uniform_info uniforms[]);

PLUGIN_END_NAMESPACE

/*
 * These pointers are only valid after calling ShadersSupported.
 */
#define SHADER_FUNCTION_LIST(proc, name) extern proc name;
#include "shaderutil.inc"
#undef SHADER_FUNCTION_LIST

#endif /* SHADER_UTIL_H */
