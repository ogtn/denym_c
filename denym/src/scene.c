#include "scene.h"
#include "renderable.h"
#include "camera.h"
#include "light.h"
#include "logger.h"


scene sceneCreate(void)
{
    scene scene = malloc(sizeof *scene);

    scene->renderableCount = 0;
    scene->maxRenderableCount = 32;
    scene->renderables = malloc(sizeof(renderable) * scene->maxRenderableCount);
    scene->camera = NULL;
    scene->light = lightCreate();

    return scene;
}


void sceneDestroy(scene scene)
{
    for(uint32_t i = 0; i < scene->renderableCount; i++)
        renderableDestroy(scene->renderables[i]);

    free(scene->renderables);
    cameraDestroy(scene->camera);
    free(scene);
}


void sceneAddRenderable(scene scene, renderable renderable)
{
    if(scene->renderableCount >= scene->maxRenderableCount)
    {
        logInfo("Max renderables updated from %u to %u",
            scene->maxRenderableCount, scene->maxRenderableCount * 2);

        scene->maxRenderableCount *= 2;
        scene->renderables = realloc(scene->renderables, sizeof(renderable) * scene->maxRenderableCount);
    }

    scene->renderables[scene->renderableCount++] = renderable;
}


void sceneDraw(scene scene, VkCommandBuffer commandBuffer)
{
    for(uint32_t i = 0; i < scene->renderableCount; i++)
        renderableDraw(scene->renderables[i], commandBuffer);
}


void sceneSetCamera(scene scene, camera camera)
{
    scene->camera = camera;
}


camera sceneGetCamera(scene scene)
{
    return scene->camera;
}


void sceneUpdate(scene scene)
{
    // TODO Here we force because camera has probably changed since last frame
    // Check camera state (keep last modified frame ?)
    // Or never provide camera trough renderable specific buffer (uniform or storage)
    // If view/projection matrices are in a global buffer, no need to update every renderable
    // Same goes for lights
    for(uint32_t i = 0; i < scene->renderableCount; i++)
    {
        renderableUpdateLighting(scene->renderables[i]);
        renderableUpdateMVP(scene->renderables[i], VK_TRUE);
    }
}
