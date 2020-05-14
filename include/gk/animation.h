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

#ifndef gk_animation_h
#define gk_animation_h
#ifdef __cplusplus
extern "C" {
#endif

#include "common.h"
#include "value.h"

struct GkAnimation;
struct GkScene;
struct GkNode;
struct GkChannel;

#define GK_INHERIT FLT_MIN

#define GK_PIVOT_CENTER_INIT  {FLT_MAX, FLT_MIN, FLT_MAX}
#define GK_PIVOT_CENTER       ((vec3)GK_PIVOT_CENTER_INIT)
#define GK_VEC3_CURRENT_INIT  {FLT_MAX, FLT_MIN, FLT_MIN}
#define GK_VEC3_CURRENT       ((vec3)GK_VEC3_CURRENT_INIT)

typedef float (*GkTimingFunc)(float t);
typedef bool    (*GkAnimFunc)(struct GkAnimation *anim,
                              GkValue            *to,
                              GkValue            *delta);

typedef bool    (*GkKFAnimFunc)(struct GkAnimation *anim,
                                struct GkChannel   *channel,
                                float              *to,
                                GkValue            *delta);


typedef enum GkInterpType {
  GK_INTERP_UNKNOWN  = 0,
  GK_INTERP_LINEAR   = 1,
  
  GK_INTERP_BEZIER   = 2,
  GK_INTERP_CARDINAL = 3,
  GK_INTERP_HERMITE  = 4,
  GK_INTERP_BSPLINE  = 5,
  GK_INTERP_STEP     = 6,

  GK_INTERP_MAXLEN   = 255
} GkInterpType;

typedef struct GkAnimation {
  GkScene     *scene;
  GkNode      *node;
  GkValue     *from;
  GkValue     *to;
  GkValue     *delta;
  void        *data;
  GkValue      curr;
  GkAnimFunc   fnAnimator;
  /* GkKFAnimFunc fnKfAnimator; */
  GkTimingFunc fnTiming;
  size_t       dataSize;
  double       duration;
  double       beginTime;
  bool         isReverse;
  bool         isKeyFrame;
  bool         autoReverse;
  bool         autoFree;
  uint32_t     nRepeat;
  uint32_t     nPlayed;
} GkAnimation;

/* TODO: */
GK_EXPORT
GkAnimation*
gkBasicAnimation(const char *path,
                 GkValue    *from,
                 GkValue    *to,
                 double      duration);

GK_EXPORT
GkAnimation*
gkRotateAnimation(float  to,
                  vec3   pivot,
                  vec3   axis,
                  double duration);

GK_EXPORT
GkAnimation*
gkScaleAnimation(vec3 to, double duration);

GK_EXPORT
GkAnimation*
gkTranslateAnimation(vec3 to, double duration);

GK_EXPORT
void
gkAddAnimation(struct GkScene *scene,
               struct GkNode  *node,
               GkAnimation    *anim);

GK_EXPORT
void
gkRemoveAnimation(GkNode *node, GkAnimation *anim);

GK_EXPORT
void
gkInterpolateChannel(struct GkAnimation * __restrict anim,
                     struct GkChannel   * __restrict ch,
                     double                          time,
                     float                           t,
                     bool                            isReverse);

#ifdef __cplusplus
}
#endif
#endif /* gk_animation_h */
