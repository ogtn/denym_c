#ifndef _denym_h_
#define _denym_h_


#include <stdint.h>

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Weverything"
#define CGLM_FORCE_LEFT_HANDED
#define CGLM_FORCE_DEPTH_ZERO_TO_ONE
#include <cglm/cglm.h>
#pragma clang diagnostic pop


typedef struct geometry_t* geometry;
typedef struct renderable_t* renderable;
typedef struct texture_t* texture;


typedef struct modelViewProj
{
    mat4 model;
    mat4 view;
    mat4 projection;
} modelViewProj;


typedef struct geometryCreateParams
{
	uint32_t vertexCount;
	uint16_t indexCount;
	uint16_t __padding;
	float *positions2D;
	float *positions3D;
	float *colors;
	float *texCoords;
	uint16_t *indices;
} geometryCreateParams;


typedef struct renderableCreateParams
{
	const char *textureName;
	const char *vertShaderName;
	const char *fragShaderName;
	geometry geometry;
	uint32_t useUniforms;
	uint32_t usePushConstant;
} renderableCreateParams;


int denymInit(int window_width, int window_height);

void denymTerminate(void);

int denymKeepRunning(void);

void denymRender(renderable *renderables, uint32_t renderablesCount);

void denymWaitForNextFrame(void);

geometry geometryCreate(const geometryCreateParams *createParams);

void geometryDestroy(geometry geometry);

renderable denymCreateRenderable(const renderableCreateParams *createParams);

void denymDestroyRenderable(renderable renderable);

int updateUniformsBuffer(renderable renderable, const modelViewProj *mvp);

int updatePushConstants(renderable renderable, float alpha);

float getUptime(void);


#endif
