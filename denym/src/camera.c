#include "camera.h"
#include "core.h"
#include "logger.h"


camera cameraCreatePerspective(float fov, float near, float far)
{
    camera camera = calloc(1, sizeof *camera);

    camera->type = CAMERA_TYPE_PERSPECTIVE;
    camera->fov = fov;
    camera->near = near;
    camera->far = far;

    float aspect = (float)engine.framebufferWidth / (float)engine.framebufferHeight;

	glm_perspective(glm_rad(fov), aspect, near, far, camera->proj);
    camera->proj[1][1] *= -1;

    camera->target[0] = 1;
    cameraLookAt(camera, camera->pos, camera->target);

    return camera;
}


camera cameraCreateOrtho(float zoom, float near, float far)
{
    camera camera = calloc(1, sizeof *camera);

    camera->type = CAMERA_TYPE_ORTHOGRAPHIC;
    camera->zoom = zoom;
    camera->near = near;
    camera->far = far;

    float x = (float)engine.framebufferWidth / zoom / 2;
    float y = (float)engine.framebufferHeight / zoom / 2;

	glm_ortho(-x, x, -y, y, near, far, camera->proj);
    camera->proj[1][1] *= -1;

    camera->target[0] = 1;
    cameraLookAt(camera, camera->pos, camera->target);

    return camera;
}


void cameraDestroy(camera camera)
{
    free(camera);
}


void cameraSetFov(camera camera, float fov)
{
    if(camera->type != CAMERA_TYPE_PERSPECTIVE)
    {
        logWarning("Can't set FoV at on this type of camera");

        return;
    }

    camera->fov = glm_clamp(fov, 60, 90);
    float aspect = (float)engine.framebufferWidth / (float)engine.framebufferHeight;
	glm_perspective(glm_rad(camera->fov), aspect, camera->near, camera->far, camera->proj);
    camera->proj[1][1] *= -1;
}


void cameraFov(camera camera, float fovOffset)
{
    cameraSetFov(camera, camera->fov + fovOffset);
}


void cameraLookAt(camera camera, vec3 eye, vec3 target)
{
    static vec3 up = { 0, 0, 1 };

    glm_vec3_copy(eye, camera->pos);
    glm_vec3_copy(target, camera->target);
    glm_lookat(eye, target, up, camera->view);
}


void cameraSetZoom(camera camera, float zoom)
{
    if(camera->type != CAMERA_TYPE_ORTHOGRAPHIC)
    {
        logWarning("Can't set zoom at on this type of camera");

        return;
    }

    camera->zoom = glm_clamp(zoom, 1, 1000);
    float x = (float)engine.framebufferWidth / camera->zoom / 2;
    float y = (float)engine.framebufferHeight / camera->zoom / 2;

	glm_ortho(-x, x, -y, y, camera->near, camera->far, camera->proj);
    camera->proj[1][1] *= -1;
}


void cameraZoom(camera camera, float zoomOffset)
{
    cameraSetZoom(camera, camera->zoom + zoomOffset);
}


void cameraResize(camera camera, int width, int height)
{
    if(camera->type == CAMERA_TYPE_PERSPECTIVE)
    {
        float aspect = (float)width / (float)height;
        glm_perspective_resize(-aspect, camera->proj);
    }
    else if(camera->type == CAMERA_TYPE_ORTHOGRAPHIC)
    {
        float x = (float)width / camera->zoom / 2;
        float y = (float)height / camera->zoom / 2;

        glm_ortho(-x, x, -y, y, camera->near, camera->far, camera->proj);
        camera->proj[1][1] *= -1;
    }
    else
        logWarning("Resizing this type of camera has no effect");
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
    if(camera->type != CAMERA_TYPE_PERSPECTIVE && camera->type != CAMERA_TYPE_ORTHOGRAPHIC)
    {
        logWarning("Can't move this type of camera");

        return;
    }

    vec3 forward;
    glm_vec3_sub(camera->target, camera->pos, forward);
    forward[2] = 0;
    glm_vec3_normalize(forward);
    vec3 right = { forward[1], -forward[0] };
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
    if(camera->type != CAMERA_TYPE_PERSPECTIVE && camera->type != CAMERA_TYPE_ORTHOGRAPHIC)
    {
        logWarning("Can't move this type of camera");

        return;
    }

    vec3 forward;
    glm_vec3_sub(camera->target, camera->pos, forward);
    vec3 right = { forward[1], -forward[0] };
    vec3 up = { 0, 0, 1 };

    // minus for yaw and pitch, because we rotate CCW
    glm_vec3_rotate(forward, -yaw, up);
    glm_vec3_rotate(forward, -pitch, right);
    glm_vec3_add(camera->pos, forward, camera->target);
    cameraLookAt(camera, camera->pos, camera->target);
}
