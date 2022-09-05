#ifndef _denym_h_
#define _denym_h_


#include <stdint.h>

#define CGLM_FORCE_LEFT_HANDED
#define CGLM_FORCE_DEPTH_ZERO_TO_ONE
#include <cglm/cglm.h>


typedef struct geometry_t* geometry;
typedef struct renderable_t* renderable;


typedef struct modelViewProj
{
    mat4 model;
    mat4 view;
    mat4 projection;
} modelViewProj;


typedef struct geometryCreateInfo
{
	uint32_t vertexCount;
	uint16_t indiceCount;

	float *positions;
	float *colors;
	uint16_t *indices;
} geometryCreateInfo;


int denymInit(int window_width, int window_height);

void denymTerminate(void);

int denymKeepRunning(void);

void denymRender(renderable renderable);

void denymWaitForNextFrame(void);

geometry geometryCreate(const geometryCreateInfo *createInfo);

void geometryDestroy(geometry geometry);

renderable denymCreateRenderable(geometry geometry, const char *vertShaderName, const char *fragShaderName);

void denymDestroyRenderable(renderable renderable);

int useUniforms(renderable renderable);

int updateUniformsBuffer(renderable renderable, const modelViewProj *mvp);

int usePushConstants(renderable renderable);

int updatePushConstants(renderable renderable, float alpha);

float getUptime(void);


#endif
