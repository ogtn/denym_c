#ifndef _scene_h_
#define _scene_h_


#include "denym_common.h"
#include "light.h"


#define SCENE_MAX_P_LIGHTS       2


typedef struct scene_t
{
    renderable *renderables;
    uint32_t renderableCount;
    uint32_t maxRenderableCount;
    camera camera;
    VkBool32 hasDirectionalLight;
    dlight_t dlight;
    uint32_t plightsCount;
    plight_t plights[SCENE_MAX_P_LIGHTS];
} scene_t;


scene sceneCreate(void);

void sceneDestroy(scene scene);

void sceneAddRenderable(scene scene, renderable renderable);

void sceneDraw(scene scene, VkCommandBuffer commandBuffer);

void sceneSetCamera(scene scene, camera camera);

camera sceneGetCamera(scene scene);

void sceneUpdate(scene scene);

dlight sceneAddDirectionalLight(scene scene);

plight sceneAddPointLight(scene scene);


#endif
