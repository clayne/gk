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

#include "../../common.h"
#include "common.h"

#include <ds/forward-list.h>
#include <ds/forward-list-sep.h>
#include <string.h>
#include <stdlib.h>

extern GkTextureState gk__deftexst;

_gk_hide
void*
gkGetOrCreatState(GkContext * __restrict ctx,
                  GkGPUStateType         type) {
  GkStatesItem *sti;
  FListItem    *item;

  sti = flist_last(ctx->states);
  if (!sti) {
    sti = calloc(1, sizeof(*sti));
    flist_insert(ctx->states, sti);
  }

  item = sti->states;
  while (item) {
    GkStateBase *state;

    state = item->data;
    if (state->type == type)
      return state;

    item = item->next;
  }

  return gkCreatState(ctx, sti, type);
}

_gk_hide
void*
gkGetOrCreatStatei(GkContext * __restrict ctx,
                   GLint                  index,
                   GkGPUStateType         type) {
  GkStatesItem *sti;
  FListItem    *item;
  GkStateBase  *st;

  sti = flist_last(ctx->states);
  if (!sti) {
    sti = calloc(1, sizeof(*sti));
    flist_insert(ctx->states, sti);
  }

  item = sti->states;
  while (item) {
    GkStateBase *state;

    state = item->data;
    if (state->type == type
        && state->index == index
        && state->indexed)
      return state;

    item = item->next;
  }

  st = gkCreatState(ctx, sti, type);
  st->indexed = true;

  return st;
}

_gk_hide
void*
gkGetOrCreatTexState(GkContext * __restrict ctx,
                     uint32_t               index,
                     GLenum                 target) {
  GkStatesItem *sti;
  FListItem    *item;

  sti = flist_last(ctx->states);
  if (!sti) {
    sti = calloc(1, sizeof(*sti));
    flist_insert(ctx->states, sti);
  }

  item = sti->states;
  while (item) {
    GkStateBase *state;

    state = item->data;
    if (state->type == GK_GPUSTATE_TEXTURE
        && state->index == index) {
      GkTextureState *texState;
      texState = (GkTextureState *)state;
      if (texState->target == target)
        return state;
    }
    item = item->next;
  }

  return gkCreatTexState(ctx, sti, index, target);
}

_gk_hide
GkStateBase*
gkCreatState(GkContext    * __restrict ctx,
             GkStatesItem * __restrict sti,
             GkGPUStateType            type) {
  struct stateMap { size_t st; size_t len; };
  GkStateBase     *state;
  GkGPUStates     *curr;
  void            **pptr, *ptr;
  char            *pCurr;
  struct stateMap  stm;

  curr = ctx->currState;

  if (type >= GK_GPUSTATE_COUNT)
    return NULL;

  stm = ((struct stateMap[]){
    {offsetof(GkGPUStates, depth),     sizeof(GkDepthState)},
    {offsetof(GkGPUStates, blend),     sizeof(GkBlendState)},
    {offsetof(GkGPUStates, tex),       sizeof(GkTextureState)},
    {offsetof(GkGPUStates, output),    sizeof(GkOutputState)},
    {offsetof(GkGPUStates, face),      sizeof(GkFaceState)},
    {offsetof(GkGPUStates, frame),     sizeof(GkFramebuffState)},
  }[type - 1]);

  pCurr = (char *)curr;
  pptr  = (void **)(pCurr + stm.st);
  ptr   = *pptr;

  state = calloc(1, stm.len);
  memcpy(state, ptr, stm.len);

  state->type = type;
  state->prev = ptr;

  sti->isempty = false;
  flist_sp_insert(&sti->states, state);

  *pptr = state;

  return state;
}

_gk_hide
GkStateBase*
gkCreatTexState(GkContext    * __restrict ctx,
                GkStatesItem * __restrict sti,
                uint32_t                  unit,
                GLenum                    target) {
  GkTextureState *state, *prev;
  GkGPUStates    *curr;
  HTable         *texu;
  size_t          stlen;

  stlen  = sizeof(GkTextureState);
  curr   = ctx->currState;
  state  = calloc(1, stlen);
  prev   = NULL;

  if ((texu = hash_get(curr->tex, DS_ITOP(unit)))) {
    prev = hash_get(texu, DS_ITOP(target));
  } else {
    texu = hash_new(NULL, ds_hashfn_ui32p, ds_cmp_ui32p, 4);
    hash_set(curr->tex, DS_ITOP(unit), texu);
  }

  /* bind default */
  if (!prev) {
    prev             = calloc(1, stlen);
    prev->base.index = unit;
    prev->base.type  = GK_GPUSTATE_TEXTURE;
    prev->target     = target;
    prev->texunit    = unit;
  }

  memcpy(state, prev, stlen);
  state->base.prev = &prev->base;

  hash_set(texu, DS_ITOP(target), state);

  sti->isempty = false;
  flist_sp_insert(&sti->states, state);

  return &state->base;
}
