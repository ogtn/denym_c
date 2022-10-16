#ifndef _scene_h_
#define _scene_h_


#include "denym_common.h"


#define SCENE_MAX_RENDERABLES 16


typedef struct scene_t
{
    renderable renderables[SCENE_MAX_RENDERABLES];
    uint32_t renderableCount;
    camera camera;
} scene_t;


scene sceneCreate(void);

void sceneDestroy(scene scene);

void sceneAddRenderable(scene scene, renderable renderable);

void sceneDraw(scene scene, VkCommandBuffer commandBuffer);

void sceneSetCamera(scene scene, camera camera);

camera sceneGetCamera(scene scene);


#endif
