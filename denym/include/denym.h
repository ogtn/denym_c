#ifndef _denym_h_
#define _denym_h_


#include <stdint.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#define CGLM_FORCE_DEPTH_ZERO_TO_ONE
#include <cglm/cglm.h>
#pragma clang diagnostic pop


typedef struct geometry_t *geometry;
typedef struct geometryParams_t *geometryParams;
typedef struct renderable_t *renderable;
typedef struct texture_t *texture;
typedef struct scene_t *scene;
typedef struct camera_t *camera;
typedef struct shader_t *shader;
typedef struct sprite_t *sprite;
#include "material.h"
#include "input.h"
#include "light.h"


typedef struct renderableCreateParams
{
	const char *textureName;
	const char *vertShaderName;
	const char *fragShaderName;
	geometry geometry;
	material material;
	uint32_t sendMVP;
	uint32_t compactMVP;
	uint32_t pushConstantSize;
	uint32_t useWireFrame;
	uint32_t sendMVPAsPushConstant;
	uint32_t sendMVPAsStorageBuffer;
	uint32_t useNearestSampler;
	uint32_t sendLigths;
} renderableCreateParams;


int denymInit(int window_width, int window_height);

void denymTerminate(void);

int denymKeepRunning(input input);

float denymGetTimeSinceLastFrame(void);

void denymRender(void);

void denymWaitForNextFrame(void);

geometryParams geometryCreateParameters(uint32_t vertexCount, uint32_t indexCount);

int geometryParamsAddIndices16(geometryParams params, uint16_t *indices);

int geometryParamsAddIndices32(geometryParams params, uint32_t *indices);

int geometryParamsAddAttribVec2(geometryParams params, float *data);

int geometryParamsAddAttribVec3(geometryParams params, float *data);

#define geometryParamsAddPositions2D geometryParamsAddAttribVec2

#define geometryParamsAddPositions3D geometryParamsAddAttribVec3

#define geometryParamsAddColorsRGB geometryParamsAddAttribVec3

#define geometryParamsAddTexCoords geometryParamsAddAttribVec2

#define geometryParamsAddNormals geometryParamsAddAttribVec3

geometry geometryCreate(const geometryParams params);

void geometryDestroy(geometry geometry);

renderable renderableCreate(const renderableCreateParams *params, uint32_t instanceCount);

void renderableDestroy(renderable renderable);

int renderableUpdatePushConstant(renderable renderable, void *value);

void renderableSetPosition(renderable renderable, float x, float y, float z);

void renderableSetPositionV(renderable renderable, vec3f *position);

void renderableMoveV(renderable renderable, vec3f *move);

void renderableRotateAxis(renderable renderable, float angle, vec3f *axis);

void renderableRotateX(renderable renderable, float angle);

void renderableRotateY(renderable renderable, float angle);

void renderableRotateZ(renderable renderable, float angle);

void renderableSetScale(renderable renderable, float x, float y, float z);

void renderableSetScaleV(renderable renderable, vec3f *scale);

void renderableScaleV(renderable renderable, vec3f *scale);

void renderableSetMatrixInstance(renderable renderable, mat4 matrix, uint32_t instanceId);

float getUptime(void);

renderable modelLoad(const char *objFile, renderableCreateParams *renderableParams, uint32_t instancesCount, uint32_t useIndices, uint32_t useTexCoords, uint32_t useNormals);

renderable primitiveCreateGrid(float size, uint32_t level);

renderable primitiveCreateCube(float size, uint32_t subdivisions, renderableCreateParams *params, uint32_t instanceCount, uint32_t useTexCoords, uint32_t useNormals);

renderable primitiveCreateSphere(float radius, uint32_t subdivisions, renderableCreateParams *params, uint32_t instanceCount, uint32_t useTexCoords, uint32_t useNormals);

sprite spriteCreate(const char *textureName, float sizeU, float sizeV, uint32_t spriteCountU, uint32_t spriteCountV);

void spriteSetSpriteCoordinates(sprite sprite, uint32_t u, uint32_t v);

void spriteSetPosition(sprite sprite, float x, float y);

void spriteFlip(sprite sprite, int verticalAxis, int horizontalAxis);

scene denymGetScene(void);

void sceneSetCamera(scene scene, camera camera);

dlight sceneAddDirectionalLight(scene scene);

plight sceneAddPointLight(scene scene);

camera cameraCreatePerspective(float fov, float near, float far);

camera cameraCreateOrtho(float zoom, float near, float far);

void cameraSetFov(camera camera, float fov);

void cameraFov(camera camera, float fovOffset);

void cameraSetZoom(camera camera, float zoom);

void cameraZoom(camera camera, float zoomOffset);

void cameraLookAt(camera camera, vec3 eye, vec3 target);

void cameraMove(camera camera, float x, float y, float z);

void cameraRotate(camera camera, float yaw, float pitch, float roll);


#endif
