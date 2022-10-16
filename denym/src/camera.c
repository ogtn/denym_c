#include "camera.h"
#include "logger.h"


camera cameraCreatePerspective(float fov, float aspect, float near, float far)
{
    camera camera = malloc(sizeof *camera);

	glm_perspective(glm_rad(fov), aspect, near, far, camera->proj);
    camera->proj[1][1] *= -1;
    camera->type = CAMERA_TYPE_3D;

    return camera;
}


void cameraDestroy(camera camera)
{
    free(camera);
}


void cameraLookAt(camera camera, vec3 eye, vec3 target)
{
    static vec3 up = { 0, 0, 1 };

    if(camera->type != CAMERA_TYPE_3D)
    {
        logWarning("Can't set look at on this type of camera");

        return;
    }

    glm_lookat(eye, target, up, camera->view);
}
