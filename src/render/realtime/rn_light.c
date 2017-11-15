/*
 * Copyright (c), Recep Aslantas.
 *
 * MIT License (MIT), http://opensource.org/licenses/MIT
 * Full license can be found in the LICENSE file
 */

#include "../../../include/gk/gk.h"
#include "../../../include/gk/material.h"
#include "../../../include/gk/prims/cube.h"
#include "../../default/def_effect.h"
#include "../../default/def_light.h"
#include "../../matrix.h"

#include "rn_light.h"
#include "rn_pass.h"
#include "rn_prim.h"

void
gkRenderPrimPerLight(GkScene     * __restrict scene,
                     GkPrimitive * __restrict prim,
                     GkProgram   * __restrict prog) {
  GkLight *firstLight, *light;
  
  light = (GkLight *)scene->lights;
  if (!light) {
    light             = gk_def_lights();
    light->isvalid    = false;
    scene->lightCount = 1;
  }

  firstLight = light;

  do {
    if (light != firstLight) {
      glDepthFunc(GL_EQUAL);
      glEnable(GL_BLEND);
      glBlendEquation(GL_FUNC_ADD);
      glBlendFunc(GL_ONE, GL_ONE);
    }

    if (light != prog->lastLight) {
      if (light->node) {
        GkNode           *node;
        GkFinalTransform *ftr;

        node = light->node;
        ftr  = node->trans->ftr;

        gkUniformSingleLight(scene,
                             light,
                             prog,
                             ftr->mv);
      } else {
        gkUniformSingleLight(scene,
                             light,
                             prog,
                             GLM_MAT4_IDENTITY);
      }

      gkUniform1ui(prog, "lightType", light->type);
      prog->lastLight = light;
    }

    gkRenderPrim(scene, prim);

    light = (GkLight *)light->ref.next;
  } while (light);

  glDepthFunc(GL_LESS);
  glDisable(GL_BLEND);
}

void
gkRenderTranspPrimPerLight(GkScene     * __restrict scene,
                           GkPrimitive * __restrict prim,
                           GkProgram   * __restrict prog) {
  GkLight *firstLight, *light;

  light = (GkLight *)scene->lights;
  if (!light) {
    light             = gk_def_lights();
    light->isvalid    = false;
    scene->lightCount = 1;
  }

  firstLight = light;

  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  do {
    if (light != firstLight) {
      glDepthFunc(GL_LEQUAL);
      glBlendEquation(GL_FUNC_ADD);
      glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    }

    if (light != prog->lastLight) {
      if (light->node) {
        GkNode           *node;
        GkFinalTransform *ftr;

        node = light->node;
        ftr  = node->trans->ftr;

        gkUniformSingleLight(scene,
                             light,
                             prog,
                             ftr->mv);
      } else {
        gkUniformSingleLight(scene,
                             light,
                             prog,
                             GLM_MAT4_IDENTITY);
      }

      gkUniform1ui(prog, "lightType", light->type);
      prog->lastLight = light;
    }

    gkRenderPrim(scene, prim);

    light = (GkLight *)light->ref.next;
  } while (light);

  glDepthFunc(GL_LESS);
  glDisable(GL_BLEND);
}
