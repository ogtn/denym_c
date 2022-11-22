#ifndef _scene_h_
#define _scene_h_


#include "denym_common.h"


typedef struct scene_t
{
    renderable *renderables;
    uint32_t renderableCount;
    uint32_t maxRenderableCount;
    camera camera;
} scene_t;


scene sceneCreate(void);

void sceneDestroy(scene scene);

void sceneAddRenderable(scene scene, renderable renderable);

void sceneDraw(scene scene, VkCommandBuffer commandBuffer);

void sceneSetCamera(scene scene, camera camera);

camera sceneGetCamera(scene scene);

void sceneUpdate(scene scene);


#endif
