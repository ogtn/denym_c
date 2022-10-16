#include "scene.h"
#include "renderable.h"
#include "camera.h"
#include "logger.h"


scene sceneCreate(void)
{
    scene scene = malloc(sizeof *scene);

    scene->renderableCount = 0;
    scene->camera = NULL;

    return scene;
}


void sceneDestroy(scene scene)
{
    for(uint32_t i = 0; i < scene->renderableCount; i++)
        denymDestroyRenderable(scene->renderables[i]);

    cameraDestroy(scene->camera);
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


void sceneSetCamera(scene scene, camera camera)
{
    scene->camera = camera;
}


camera sceneGetCamera(scene scene)
{
    return scene->camera;
}
