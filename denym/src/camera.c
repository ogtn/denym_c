#include "camera.h"
#include "core.h"
#include "logger.h"


camera cameraCreatePerspective(float fov, float near, float far)
{
    camera camera = calloc(1, sizeof *camera);

    float aspect = (float)engine.framebufferWidth / (float)engine.framebufferHeigt;
	glm_perspective(glm_rad(fov), aspect, near, far, camera->proj);
    camera->proj[1][1] *= -1;
    camera->type = CAMERA_TYPE_3D;

    camera->target[0] = 1;
    cameraLookAt(camera, camera->pos, camera->target);

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

    glm_vec3_copy(eye, camera->pos);
    glm_vec3_copy(target, camera->target);
    glm_lookat(eye, target, up, camera->view);
}


void cameraResize(camera camera, int with, int height)
{
    if(camera->type == CAMERA_TYPE_3D)
    {
        float aspect = (float)with / (float)height;
        glm_perspective_resize(-aspect, camera->proj);
    }
    else
        logWarning("Resizing non perpective camera has no effect");
}


void cameraGetView(camera camera, mat4 out)
{
    glm_mat4_copy(camera->view, out);
}


void cameraGetProj(camera camera, mat4 out)
{
    glm_mat4_copy(camera->proj, out);
}


void cameraMove(camera camera, float x, float y, float z)
{
    if(camera->type != CAMERA_TYPE_3D)
    {
        logWarning("Can't move this type of camera");

        return;
    }

    vec3 forward;
    glm_vec3_sub(camera->target, camera->pos, forward);
    forward[2] = 0;
    glm_vec3_normalize(forward);
    vec3 right = { -forward[1], forward[0] };
    vec3 up = { 0, 0, 1 };

    glm_vec3_scale(forward, x, forward);
    glm_vec3_scale(right, y, right);
    glm_vec3_scale(up, z, up);

    // TODO: limit speed here
    vec3 move;
    glm_vec3_add(forward, right, move);
    glm_vec3_add(move, up, move);

    glm_vec3_add(camera->pos, move, camera->pos);
    glm_vec3_add(camera->target, move, camera->target);
    cameraLookAt(camera, camera->pos, camera->target);
}


void cameraRotate(camera camera, float yaw, float pitch, float roll)
{
    if(camera->type != CAMERA_TYPE_3D)
    {
        logWarning("Can't move this type of camera");

        return;
    }

    vec3 forward;
    glm_vec3_sub(camera->target, camera->pos, forward);
    vec3 right = { -forward[1], forward[0] };
    vec3 up = { 0, 0, 1 };

    glm_vec3_rotate(forward, yaw, up);
    glm_vec3_rotate(forward, pitch, right);
    glm_vec3_add(camera->pos, forward, camera->target);
    cameraLookAt(camera, camera->pos, camera->target);
}
