/*
 * Copyright (c), Recep Aslantas.
 *
 * MIT License (MIT), http://opensource.org/licenses/MIT
 * Full license can be found in the LICENSE file
 */

#ifndef gk_shader_h
#define gk_shader_h
#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"
#include <cglm/cglm.h>
#include <OpenGL/gl3.h>
#include <stdio.h>

typedef struct GkShader {
  struct GkShader *next;
  GLuint           shaderId;
  GLuint           shaderType;
  GLboolean        isValid;
} GkShader;

GLuint
gkShaderLoadFromFile(GLenum shaderType,
                     const char * __restrict path);

int
gkShaderLoadFromFolder(const char * __restrict path,
                       GkShader ** __restrict dest);

GLuint
gkShaderLoad(GLenum shaderType,
             const char * __restrict source);

GLuint
gkShaderLoadN(GLenum  shaderType,
              char   *source[],
              size_t  count);

void
gkAttachShaders(GLuint program,
                GkShader * __restrict shaders);

void
gkShaderLogInfo(GLuint shaderId,
                FILE * __restrict file);

void
gkUniformMat4(GLint location, mat4 matrix);

GkShader*
gkShaderByName(const char *name);

void
gkShaderSetName(GkShader *shader, const char *name);

#ifdef __cplusplus
}
#endif
#endif /* gk_shader_h */
