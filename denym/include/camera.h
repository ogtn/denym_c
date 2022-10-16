#ifndef _camera_h_
#define _camera_h_


#include "denym_common.h"


typedef enum cameraType
{
    CAMERA_TYPE_3D,
    CAMERA_TYPE_2D
} cameraType;


typedef struct camera_t
{
    mat4 view;
	mat4 proj;
    cameraType type;
} camera_t;


camera cameraCreatePerspective(float fov, float aspect, float near, float far);

void cameraDestroy(camera camera);

void cameraLookAt(camera camera, vec3 eye, vec3 target);


#endif
