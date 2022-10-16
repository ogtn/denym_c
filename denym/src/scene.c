#include "scene.h"
#include "renderable.h"
#include "logger.h"


scene sceneCreate(void)
{
    scene scene = malloc(sizeof *scene);

    scene->renderableCount = 0;

    return scene;
}


void sceneDestroy(scene scene)
{
    for(uint32_t i = 0; i < scene->renderableCount; i++)
        denymDestroyRenderable(scene->renderables[i]);

    free(scene);
}


void sceneAddRenderable(scene scene, renderable renderable)
{
    if(scene->renderableCount >= SCENE_MAX_RENDERABLES)
    {
        logWarning("Max renderables reached in the scene");

        return;
    }

    scene->renderables[scene->renderableCount++] = renderable;
}


void sceneDraw(scene scene, VkCommandBuffer commandBuffer)
{
    for(uint32_t i = 0; i < scene->renderableCount; i++)
        renderableDraw(scene->renderables[i], commandBuffer);
}
