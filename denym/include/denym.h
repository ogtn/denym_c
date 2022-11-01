#ifndef _denym_h_
#define _denym_h_


#include <stdint.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#define CGLM_FORCE_LEFT_HANDED
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


typedef struct renderableCreateParams
{
	const char *textureName;
	const char *vertShaderName;
	const char *fragShaderName;
	geometry geometry;
	uint32_t sendMVP;
	uint32_t compactMVP;
	uint32_t pushConstantSize;
	uint32_t useWireFrame;
	uint32_t sendMVPAsPushConstant;
	uint32_t sendMVPAsStorageBuffer;
} renderableCreateParams;


int denymInit(int window_width, int window_height);

void denymTerminate(void);

int denymKeepRunning(void);

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

renderable renderableCreate(const renderableCreateParams *createParams);

void renderableDestroy(renderable renderable);

int renderableUpdatePushConstant(renderable renderable, void *value);

void renderableSetMatrix(renderable renderable, mat4 matrix);

float getUptime(void);

renderable modelLoad(const char *objFile, renderableCreateParams *renderableParams, int useIndices, int useNormals);

scene denymGetScene(void);

void sceneSetCamera(scene scene, camera camera);

camera cameraCreatePerspective(float fov, float near, float far);

void cameraLookAt(camera camera, vec3 eye, vec3 target);

#endif
