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

#ifndef gk_pass_h
#define gk_pass_h

#include "common.h"
#include "program.h"
#include "material.h"
#include "geom-types.h"
#include <ds/forward-list.h>
#include <stdbool.h>

struct GkScene;
struct GkContext;
struct GkPrimitive;
struct GkLight;

typedef struct GkClearOp {
  vec4    *color;
  float    depth;
  bool     clearColor:1;
  bool     clearDepth:1;
  bool     enabled;
} GkClearOp;

typedef struct GkBlendOp {
  bool     enabled;
  bool     separate;
  GLenum   src;
  GLenum   dst;
} GkBlendOp;

typedef struct GkDepthTest {
  bool        enabled;
  GLenum      func;
  GLboolean   mask;
} GkDepthTest;

/* GL_COLOR_ATTACHMENT[n] */
typedef struct GkColorOutput {
  GLuint                buffId;
  GLsizei               width;
  GLsizei               height;
  GLenum                attachment;
  GLenum                drawIndex;
  GkClearOp            *clear;
  GkBlendOp            *blend;
  struct GkColorOutput *next;
} GkColorOutput;

typedef struct GkOutput {
  GLuint         fbo;
  GLuint         depth;   /* GL_DEPTH_ATTACHMENT   */
  GLuint         stencil; /* GL_STENCIL_ATTACHMENT */
  GkColorOutput *color;   /* GL_COLOR_ATTACHMENT0  */
  GkClearOp     *clear;
  GkBlendOp     *blend;
  uint32_t       colorCount;
  bool           cleared;
} GkOutput;

typedef struct GkPass {
  GkPipeline     *prog;
  FListItem     *states;
  GkOutput      *output;
  struct GkPass *inPasses;
  struct GkPass *outPass;
  struct GkPass *next;
  GkClearOp     *clear;
  GkBlendOp     *blend;
  GkDepthTest   *depthTest;
  bool           noLights;
  bool           noMaterials;
} GkPass;

GK_EXPORT
GkOutput*
gkDefaultRenderOut(void);

GK_EXPORT
GkOutput*
gkCurrentOutput(struct GkContext * __restrict ctx);

GK_EXPORT
GkPass*
gkGetOrCreatPass(struct GkScene    *scene,
                 struct GkLight    *light,
                 struct GkPrimInst *primInst,
                 GkMaterial        *mat);

GK_EXPORT
void
gkBindPass(struct GkScene * __restrict scene,
           GkPass         * __restrict pass);

GK_EXPORT
GkPass*
gkAllocPass(struct GkContext * __restrict ctx);

GK_EXPORT
GkOutput*
gkAllocOutput(struct GkContext * __restrict ctx);

GK_EXPORT
void
gkBindOutput(struct GkScene *scene,
             GkOutput       *output);

GK_EXPORT
void
gkBindOutputFor(struct GkContext * __restrict ctx, GkOutput *output);

GK_EXPORT
void
gkBindDefaultOutput(struct GkScene *scene);

GK_EXPORT
void
gkAddDepthTarget(struct GkScene *scene,
                  GkPass         *pass);

GK_EXPORT
void
gkAddDepthTexTarget(struct GkScene *scene,
                     GkPass         *pass,
                     GkSize          size);

GK_EXPORT
void
gkAddDepthTexArrayTarget(struct GkScene *scene, GkPass *pass, GLsizei len);

GK_EXPORT
void
gkAddDepthCubeTexTarget(struct GkScene *scene, GkPass *pass, float size);

GK_EXPORT
GkColorOutput*
gkGetRenderTarget(GkPass *rt, int32_t index);

GK_EXPORT
void
gkBindRenderTargetTo(struct GkScene *scene,
                     GkPass         *pass,
                     int32_t         targetIndex,
                     GkPipeline      *prog,
                     int32_t         texUnit,
                     const char      *uniformName);

GK_EXPORT
void
gkBindDepthTexTo(struct GkScene *scene,
                 GkPass         *pass,
                 GkPipeline      *prog,
                 int32_t         texUnit,
                 const char     *uniformName);

GK_EXPORT
void
gkBindDepthTexArrayTo(struct GkScene *scene,
                      GkPass         *pass,
                      GkPipeline      *prog,
                      int32_t         texUnit,
                      const char     *uniformName);

GK_EXPORT
GLuint
gkAddRenderTarget(struct GkScene *scene,
                  GkPass         *pass,
                  GLenum          internalformat,
                  GLenum          format,
                  GLenum          type);

GK_EXPORT
GLuint
gkAddRenderTargetRB(struct GkScene *scene,
                    GkPass         *pass);

GK_EXPORT
GLuint
gkAddRenderTargetEx(struct GkScene *scene,
                    GkPass *pass,
                    GLenum  internalFormat,
                    GLenum  format,
                    GLsizei width,
                    GLsizei height,
                    GLenum  type);

GK_EXPORT
GLuint
gkAddRenderTargetRBEx(struct GkScene *scene,
                      GkPass *pass,
                      GLenum  internalFormat,
                      GLsizei width,
                      GLsizei height);

GK_EXPORT
void
gkClearColor(GkColorOutput *colorOutput);

GK_EXPORT
void
gkClearColorAt(GkOutput *output, int32_t buffIndex);

GK_EXPORT
void
gkClearColors(GkOutput *output);

#endif /* gk_pass_h */
