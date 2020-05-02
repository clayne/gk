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

#ifndef src_gpustate_common_h
#define src_gpustate_common_h

#include "../../../include/gk/gk.h"
#include "../../../include/gk/gpu_state.h"
#include "../../../include/gk/pass.h"

#include <ds/hash.h>

typedef enum GkGPUStateType {
  GK_GPUSTATE_DEPTH      = 1,
  GK_GPUSTATE_BLEND      = 2,
  GK_GPUSTATE_TEXTURE    = 3,
  GK_GPUSTATE_OUTPUT     = 4,
  GK_GPUSTATE_CULLFACE   = 5,
  GK_GPUSTATE_FRAME      = 6,

  GK_GPUSTATE_COUNT
} GkGPUStateType;

typedef struct GkStateBase {
  struct GkStateBase *prev;
  struct GkStateBase *next;
  struct GkStateBase *allbuf;
  GkGPUStateType      type;
  int32_t             index;
  bool                indexed;
} GkStateBase;

typedef struct GkDepthState {
  GkStateBase base;
  bool        test;
  GLenum      func;
  GLboolean   mask;
} GkDepthState;

typedef struct GkFaceState {
  GkStateBase base;
  bool        cull;
  GLenum      face;
  GLenum      frontFace;
} GkFaceState;

typedef struct GkBlendState {
  GkStateBase base;
  bool        blend;
  GLenum      eq;
  GLenum      src;
  GLenum      dst;
} GkBlendState;

typedef struct GkTextureState {
  GkStateBase base;
  GLuint      texunit;
  GLenum      target;
  GLuint      texid;
} GkTextureState;

typedef struct GkFramebuffState {
  GkStateBase base;
  GLenum      readbuff;
  GLenum      drawbuff;
} GkFramebuffState;

typedef struct GkRenderOutState {
  GkStateBase base;
  GkOutput   *output;
} GkOutputState;

typedef struct GkStatesItem {
  FListItem *states;
  bool       isempty;
} GkStatesItem;

typedef struct GkGPUStates {
  GkDepthState     *depth;
  GkBlendState     *blend;
  GkOutputState    *output;
  GkFaceState      *face;
  GkFramebuffState *frame;
  HTable           *tex;
  GkPipeline        *prog;
  GLuint            activeTex;
} GkGPUStates;

typedef void (*GkGPUApplyStateFn)(GkContext   * __restrict ctx,
                                  GkStateBase * __restrict st);
typedef GkStateBase* (*GkGPUStateCreatFn)(GkContext * __restrict ctx);

_gk_hide
void*
gkGetOrCreatState(GkContext * __restrict ctx,
                  GkGPUStateType         type);

_gk_hide
void*
gkGetOrCreatStatei(GkContext * __restrict ctx,
                   GLint                  index,
                   GkGPUStateType         type);

_gk_hide
GkStateBase*
gkCreatState(GkContext    * __restrict ctx,
             GkStatesItem * __restrict sti,
             GkGPUStateType            type);

_gk_hide
void*
gkGetOrCreatTexState(GkContext * __restrict ctx,
                     uint32_t               index,
                     GLenum                 target);

_gk_hide
GkStateBase*
gkCreatTexState(GkContext    * __restrict ctx,
                GkStatesItem * __restrict sti,
                uint32_t                  index,
                GLenum                    target);

#endif /* src_gpustate_common_h */
