/*
 * This file is part of the gk project (https://github.com/recp/gk)
 * Copyright (c) Recep Aslantas.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "common.h"
#include "../include/gk/shader.h"

#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <unistd.h>
#include <assert.h>
#include <limits.h>

#include "fs_utils.h"

void
gkShaderLogInfo(GLuint shaderId,
                FILE * __restrict file) {
  char * infoLog;
  GLint  infoLogLen;

  glGetShaderiv(shaderId,
                GL_INFO_LOG_LENGTH,
                &infoLogLen);

  infoLog = malloc(sizeof(*infoLog) * infoLogLen + 1);

  glGetShaderInfoLog(shaderId,
                     infoLogLen,
                     NULL,
                     infoLog);

  fprintf(file,
          " -- Info Log For Shader: %d -- \n",
          shaderId);

  fwrite(infoLog,
         1,
         infoLogLen,
         file);

  fprintf(file, " --------------------- \n");

  free(infoLog);
}

GLuint
gkShaderLoad(GLenum shaderType,
             const char * source) {
  FILE  *logFile;
  GLuint shaderId;
  GLint  status;

  logFile  = stderr;
  shaderId = glCreateShader(shaderType);

  if(shaderId == 0) {
    fprintf(logFile, "Couldn't create shader!");
    return shaderId;
  }

  glShaderSource(shaderId,
                 1,
                 (const GLchar **)&source,
                 NULL);

  glCompileShader(shaderId);

  glGetShaderiv(shaderId,
                GL_COMPILE_STATUS,
                &status);

  if (status == GL_FALSE) {
    gkShaderLogInfo(shaderId, logFile);

    glDeleteShader(shaderId);
    shaderId = 0;
  }

  return shaderId;
}

GLuint
gkShaderLoadN(GLenum  shaderType,
              char   *source[],
              size_t  count) {
  char  *src;
  size_t i, len;
  GLuint ret;

  len = 0;
  for (i = 0; i < count; i++)
    len += strlen(source[i]);

  if (len == 0)
    return -1;

  src = malloc(len + 1);
  src[0] = src[len] = '\0';

  for (i = 0; i < count; i++)
    strcat(src, source[i]);

  ret = gkShaderLoad(shaderType, src);
  free(src);

  return ret;
}

GLuint
gkShaderLoadFromFile(GLenum shaderType,
                     const char * path) {
  char * source;
  GLuint shaderId;
  int    ret;

  shaderId = 0;
  ret      = fsReadfile(path, "rb", &source);

  if (ret == 0)
    shaderId = gkShaderLoad(shaderType, source);

  free(source);

  return shaderId;
}

int
gkShaderLoadFromFolder(const char * __restrict path,
                       GkShader  ** __restrict dest) {
  GkShader      *shader;
  GkShader      *lastShader;
  GkShader      *currShader;
  DIR           *dir;
  struct dirent *ent;
  char           cwdir[PATH_MAX];
  int            count;

  count      = 0;
  shader     = NULL;
  currShader = NULL;
  lastShader = NULL;

  getcwd(cwdir, 0);
  if (chdir(path) < 0) {
    fprintf(stderr, "Unable change current work dir to specified path!");
    return 0;
  }

  if ((dir = opendir(path))) {
    while ((ent = readdir(dir))) {
      const char *fname;
      const char *ext;
      const char *dot;
      GLuint      shaderType;

      fname = ent->d_name;

      dot = strrchr(fname, '.');
      if (!dot || dot == fname || strcmp(fname, "..") == 0)
        continue;

      ext = dot + 1;

      if (strcmp(ext, "vert") == 0) {
        shaderType = GL_VERTEX_SHADER;
      } else if (strcmp(ext, "frag") == 0) {
        shaderType = GL_FRAGMENT_SHADER;
      } else if (strcmp(ext, "geom") == 0) {
        shaderType = GL_GEOMETRY_SHADER;
      } else if (strcmp(ext, "tesc") == 0) {
        shaderType = GL_TESS_CONTROL_SHADER;
      } else if (strcmp(ext, "tese") == 0) {
        shaderType = GL_TESS_EVALUATION_SHADER;
      } else {
        continue;
      }

      currShader = calloc(1, sizeof(*shader));
      currShader->shaderType = shaderType;
      currShader->shaderId   = gkShaderLoadFromFile(shaderType, fname);
      currShader->isValid    = currShader->shaderId > 0;

      if (lastShader)
        lastShader->next = currShader;

      if (!shader)
        shader = currShader;

      lastShader = currShader;
    }

    (void)closedir(dir);
  } else {
    shader = NULL;
    count  = -1;
  }

  /* restore cwdir */
  chdir(cwdir);

  *dest = shader;

  return count;
}

void
gkAttachShaders(GLuint program,
                GkShader * __restrict shaders) {
  GkShader *shaderIter;

  for (shaderIter = shaders;
       shaderIter;
       shaderIter = shaderIter->next) {

    if (shaderIter->isValid)
      glAttachShader(program, shaderIter->shaderId);
  }
}

void
gkUniformMat4(GLint location, mat4 matrix) {
  glUniformMatrix4fv(location, 1, GL_FALSE, matrix[0]);
}
