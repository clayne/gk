/*
 * Copyright (c), Recep Aslantas.
 *
 * MIT License (MIT), http://opensource.org/licenses/MIT
 * Full license can be found in the LICENSE file
 */

#ifndef gk_light_h
#define gk_light_h

#include "common.h"
#include "color.h"
#include <cglm/cglm.h>

struct GkScene;
struct GkNode;
struct GkProgram;
struct GkColorOrTex;
struct GkCamera;
struct GkTransform;

typedef enum GkLightType {
  GK_LIGHT_TYPE_AMBIENT     = 1,
  GK_LIGHT_TYPE_DIRECTIONAL = 2,
  GK_LIGHT_TYPE_POINT       = 3,
  GK_LIGHT_TYPE_SPOT        = 4,
  GK_LIGHT_TYPE_CUSTOM      = 5
} GkLightType;

typedef enum GkLightFlags {
  GK_LIGHTF_NONE          = 0,
  GK_LIGHTF_TRANSFORMED   = 1 << 1,
  GK_LIGHTF_DISABLED      = 1 << 2
} GkLightFlags;

/* for scene, because node can have multiple light instancs */
typedef struct GkLightRef {
  struct GkLightRef *prev;
  struct GkLightRef *next;
} GkLightRef;

typedef struct GkLight {
  GkLightRef       ref;
  struct GkLight  *next;
  struct GkNode   *node;
  struct GkCamera *camera;
  const char      *name;
  vec4            *ambient;
  vec3             dir;
  vec3             defdir;
  GkLightType      type;
  GkColor          color;
  int32_t          index;
  uint8_t          isvalid;
  GkLightFlags     flags;
} GkLight;

typedef GkLight GkAmbientLight;

typedef struct GkDirectionalLight {
  GkLight base;
} GkDirectionalLight;

typedef struct GkPointLight {
  GkLight base;
  float   constAttn;
  float   linearAttn;
  float   quadAttn;
} GkPointLight;

typedef struct GkSpotLight {
  GkLight base;
  float   constAttn;
  float   linearAttn;
  float   quadAttn;
  float   falloffAngle;
  float   cutoffCosine;
  float   cutoffExp;
} GkSpotLight;

void
gkUniformLights(struct GkScene   * __restrict scene,
                struct GkProgram * __restrict prog);

void
gkUniformLight(struct GkScene   * __restrict scene,
               GkLight          * __restrict light,
               struct GkProgram * __restrict prog,
               mat4                          transView);

void
gkUniformSingleLight(struct GkScene   * __restrict scene,
                     GkLight          * __restrict light,
                     struct GkProgram * __restrict prog);

void
gkApplyTransformToLight(struct GkScene   * __restrict scene,
                        GkLight          * __restrict light,
                        struct GkProgram * __restrict prog);

void
gkShadowMatrix(struct GkScene *scene,
               GkLight        *light,
               mat4            viewProj);

struct GkTransform*
gkLightTransform(GkLight *light);

void
gkLightPos(struct GkScene *scene, GkLight *light, vec3 position);

void
gkLightDir(struct GkScene *scene, GkLight *light, vec3 dir);

void
gkLightDirWorld(struct GkScene *scene, GkLight *light, vec3 dir);

void
gkLightRotation(struct GkScene *scene,
                GkLight        *light,
                vec3            right,
                vec3            up,
                vec3            fwd);

#endif /* gk_light_h */
