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
#include "../include/gk/scene.h"
#include "../include/gk/node.h"
#include "../include/gk/opt.h"
#include "bbox/scene_bbox.h"
#include "anim/animatable.h"

#include <ds/hash.h>
#include <string.h>

uint32_t gk_nodesPerPage = 64;

static
void
gkPrepareNode(GkScene * __restrict scene,
              GkNode  * __restrict parentNode,
              GkNode  * __restrict node);

static
void
gkPrepInstSkin(GkScene * __restrict scene);

GkNode*
gkAllocNode(struct GkScene * __restrict scene) {
  GkNodePage  *np;
  GkSceneImpl *sceneImpl;
  GkNode      *node;
  size_t       i;

  sceneImpl = (GkSceneImpl *)scene;
  np        = sceneImpl->lastPage;

  if (!np || np->size == np->count)
    goto nw;

  for (i = 0; i < gk_nodesPerPage; i++) {
    node = &np->nodes[i];
    if (node->flags & GK_NODEF_NODE)
      continue;

    node->flags |= GK_NODEF_NODE;
    np->count++;
    return node;
  }

nw:
  np        = calloc(1, sizeof(*np) + gk_nodesPerPage * sizeof(GkNode));
  np->size  = gk_nodesPerPage;
  np->next  = sceneImpl->lastPage;
  np->count = 1;

  sceneImpl->lastPage = np;
  if (!sceneImpl->nodePages)
    sceneImpl->nodePages = np;

  node = &np->nodes[0];
  node->flags |= GK_NODEF_NODE;

  node->anim = gkAnimatable(node);

  return node;
}

GK_EXPORT
void
gkFreeNode(GkScene * __restrict scene,
           GkNode  * __restrict node) {
  node->flags &= ~GK_NODEF_NODE;

  /* todo: free node and all child nodes */
}

void
gkMakeNodeTransform(GkScene * __restrict scene,
                    GkNode  * __restrict node) {
  GkTransform *tr;

  if (node->trans)
    return;

  tr           = gkAllocTransform(scene);
  node->trans  = tr;
  node->flags |= GK_NODEF_HAVE_TRANSFORM;
}

static
void
gkPrepareNode(GkScene * __restrict scene,
              GkNode  * __restrict parentNode,
              GkNode  * __restrict node) {
  GkSceneImpl     *sceneImpl;
  GkCameraImpl    *camImpl;
  FListItem       *camItem;
  GkTransform     *tr;
  GkLight         *light;
  GkInstanceMorph *morper;
  bool             finalComputed;

  sceneImpl = (GkSceneImpl *)scene;
  camItem   = sceneImpl->transfCacheSlots->first;

  finalComputed = false;
  if (!(tr = node->trans))
    tr = node->trans = parentNode->trans;

  if (!GK_FLG(tr->flags, GK_TRANSF_LOCAL_ISVALID))
    gkTransformCombine(tr);

  if (parentNode && (node->flags & GK_NODEF_HAVE_TRANSFORM))
    glm_mul(parentNode->trans->world, tr->local, tr->world);

  /* TODO: */
  /* gkTransformAABB(tr, node->bbox); */

  if (node->model) {
    GkModelInst *modelInst;

    glm_vec3_scale(scene->center, sceneImpl->centercount, scene->center);

    while (camItem) {
      camImpl = camItem->data;
      gkCalcFinalTransf(scene, &camImpl->pub, tr);
      camItem = camItem->next;
    }

    finalComputed = true;
    modelInst     = node->model;

    do {
      GkPrimInst *prims;
      vec3        modelCenter;
      int32_t     i, primc;

      modelInst->trans = tr;
      glm_aabb_transform(modelInst->model->bbox, tr->world, modelInst->bbox);

      prims = modelInst->prims;
      primc = modelInst->primc;

      glm_vec3_zero(modelCenter);

      for (i = 0; i < primc; i++) {
        glm_aabb_transform(prims[i].prim->bbox, tr->world, prims[i].bbox);

        gkUpdateSceneAABB(scene, prims[i].bbox);
        prims[i].trans = tr;
      }

      glm_mat4_mulv3(tr->world,
                     modelInst->model->center,
                     1.0f,
                     modelInst->center);

      if (modelInst->addedToScene) {
        glm_vec3_sub(scene->center, modelInst->center, scene->center);
      } else {
        modelInst->addedToScene = true;
        sceneImpl->centercount++;
      }

      glm_vec3_add(scene->center, modelInst->center, scene->center);

      if ((morper = node->morpher)) {
        gkAttachMorphTo(morper->morph, modelInst);
      }

      modelInst = modelInst->next;
    } while (modelInst);

    glm_vec3_divs(scene->center, sceneImpl->centercount, scene->center);
  }

  if ((light = node->light)) {
    if (!finalComputed) {
      while (camItem) {
        camImpl = camItem->data;
        gkCalcViewTransf(scene, &camImpl->pub, tr);
        camItem = camItem->next;
      }
    }

    light->flags |= GK_LIGHTF_TRANSFORMED;
    glm_vec3_rotate_m4(tr->world, light->defdir, light->dir);
    glm_vec3_normalize(light->dir);
  }
}

GK_EXPORT
void
gkApplyTransform(GkScene * __restrict scene,
                 GkNode  * __restrict node) {
  GkTransform  *tr;
  GkNode       *mostParent;

  if (!(tr = node->trans))
    node->trans = scene->trans;

  gkPrepareNode(scene, node->parent, node);

  /* do the same for child nodes */
  if (!node->chld)
    return;

  if (!node->parent)
    glm_mul(scene->trans->world, tr->local, tr->world);

  mostParent = node->parent;
  node       = node->chld;
  while (node) {
    gkPrepareNode(scene, node->parent, node);
    while (node->chld) {
      gkPrepareNode(scene, node, node->chld);
      node = node->chld;
    }

    if (node->next) {
      node = node->next;
      continue;
    }

    node = node->parent;
    while (node) {
      if (node == mostParent)
        goto dn;
      if (node->next) {
        node = node->next;
        break;
      }
      node = node->parent;
    }
  }

dn:; /* done */

  /* TODO: optimize this */
  gkPrepInstSkin(scene);
}

GK_INLINE
void
gkPrepareView(GkScene * __restrict scene,
              GkNode  * __restrict node) {
  GkSceneImpl  *sceneImpl;
  GkCameraImpl *camImpl;
  FListItem    *camItem;
  GkTransform  *tr;
  GkLight      *light;
  bool          finalComputed;

  sceneImpl = (GkSceneImpl *)scene;
  camItem   = sceneImpl->transfCacheSlots->first;
  tr        = node->trans;

  if (!(tr->flags & GK_TRANSF_CALC_VIEW))
    return;

  finalComputed = false;
  if (node->model) {
    while (camItem) {
      camImpl = camItem->data;
      gkCalcFinalTransf(scene, &camImpl->pub, tr);
      camItem = camItem->next;
    }

    finalComputed = true;
  }

  if ((light = node->light)) {
    if (!finalComputed) {
      while (camItem) {
        camImpl = camItem->data;
        gkCalcViewTransf(scene, &camImpl->pub, tr);
        camItem = camItem->next;
      }
    }

    light->flags |= GK_LIGHTF_TRANSFORMED;
    glm_vec3_rotate_m4(tr->world, light->defdir, light->dir);
    glm_vec3_normalize(light->dir);
  }
}

GK_EXPORT
void
gkApplyView(struct GkScene * __restrict scene,
            GkNode         * __restrict node) {
  GkNodePage  *np;
  GkSceneImpl *sceneImpl;
  GkModelInst *modelInst;
  size_t       i;

  sceneImpl = (GkSceneImpl *)scene;
  np        = sceneImpl->lastPage;

  /* invalidate */
  while (np) {
    for (i = 0; i < gk_nodesPerPage; i++) {
      node = &np->nodes[i];

      /* unallocated node */
      if (!(node->flags & GK_NODEF_NODE)
          || !(modelInst = node->model))
        continue;

      while (modelInst) {
        modelInst->trans->flags |= GK_TRANSF_CALC_VIEW;
        modelInst = modelInst->next;
      }
    }

    np = np->next;
  }

  np = sceneImpl->lastPage;
  while (np) {
    for (i = 0; i < gk_nodesPerPage; i++) {
      node = &np->nodes[i];

      /* unallocated node */
      if (!(node->flags & GK_NODEF_NODE))
        continue;

      if (node->model || node->light)
        gkPrepareView(scene, node);
    }

    np = np->next;
  }
}

/* TODO: optimize this */
static
void
gkPrepInstSkin(GkScene * __restrict scene) {
  GkSceneImpl      *sceneImpl;
  FListItem        *item;
  GkNode           *node;
  GkControllerInst *ctlrInst;
  GkModelInst      *modelInst;

  GkSkin *skin;
  GkNode *joint;
  size_t  nJoints, i;

  sceneImpl = (GkSceneImpl *)scene;
  if ((item = sceneImpl->instSkins)) {
    do {
      node     = item->data;
      ctlrInst = node->controller;

      if (ctlrInst->ctlr && ctlrInst->ctlr->type == GK_CONTROLLER_SKIN) {
        skin      = (GkSkin *)ctlrInst->ctlr;
        nJoints   = skin->nJoints;
        modelInst = skin->base.source;

        if (!modelInst->joints) {
          modelInst->joints = malloc(sizeof(mat4) * skin->nJoints);
          glm_mat4_identity_array(modelInst->joints, skin->nJoints);
        }

        if (ctlrInst->joints) {
          for (i = 0; i < nJoints; i++) {
            if ((joint = ctlrInst->joints[i])) {
              glm_mat4_mulN((mat4 *[]){
                &joint->trans->world,
                &skin->invBindPoses[i],
                &skin->bindShapeMatrix
              }, 3, modelInst->joints[i]);

              if (scene->flags & GK_SCENEF_DRAW_BONES) {
                if (!modelInst->jointsToDraw)
                  modelInst->jointsToDraw = malloc(sizeof(mat4) * skin->nJoints);

                glm_mat4_copy(joint->trans->world, modelInst->jointsToDraw[i]);
              }
            }
          }
        } else if (skin->joints) {
          for (i = 0; i < nJoints; i++) {
            if ((joint = skin->joints[i])) {
              glm_mat4_mulN((mat4 *[]){
                &joint->trans->world,
                &skin->invBindPoses[i],
                &skin->bindShapeMatrix
              }, 3, modelInst->joints[i]);

              if (scene->flags & GK_SCENEF_DRAW_BONES) {
                if (!modelInst->jointsToDraw)
                  modelInst->jointsToDraw = malloc(sizeof(mat4) * skin->nJoints);

                glm_mat4_copy(joint->trans->world, modelInst->jointsToDraw[i]);
              }
            }
          }
        }

        /* TODO: optimize this */
        gkUniformJoints(scene, modelInst);
      }
    } while ((item = item->next));
  }
}
