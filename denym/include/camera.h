#ifndef _camera_h_
#define _camera_h_


#include "denym_common.h"


typedef enum cameraType
{
    CAMERA_TYPE_PERSPECTIVE,
    CAMERA_TYPE_ORTHOGRAPHIC
} cameraType;


typedef struct camera_t
{
    mat4 view;
	mat4 proj;
    cameraType type;

    vec3 pos;
    vec3 target;
    float near;
    float far;

    // perspective
    float fov;

    // orthographic
    float zoom;
} camera_t;


camera cameraCreatePerspective(float fov, float near, float far);

camera cameraCreateOrtho(float zoom, float near, float far);

void cameraDestroy(camera camera);

void cameraSetFov(camera camera, float fov);

void cameraSetZoom(camera camera, float zoom);

void cameraLookAt(camera camera, vec3 eye, vec3 target);

void cameraResize(camera camera, int width, int height);

void cameraGetView(camera camera, mat4 out);

void cameraGetProj(camera camera, mat4 out);

void cameraMove(camera camera, float x, float y, float z);

void cameraRotate(camera camera, float yaw, float pitch, float roll);


#endif
