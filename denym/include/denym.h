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
typedef struct geometryParams_t *geometryParams;
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
	uint32_t indexCount;
	float *positions2D;
	float *positions3D;
	float *colors;
	float *texCoords;
	uint16_t *indices_16;
	uint32_t *indices_32;
} geometryCreateParams;


typedef struct renderableCreateParams
{
	const char *textureName;
	const char *vertShaderName;
	const char *fragShaderName;
	geometry geometry;
	size_t uniformSize;
	uint32_t usePushConstant;
} renderableCreateParams;


int denymInit(int window_width, int window_height);

void denymTerminate(void);

int denymKeepRunning(void);

void denymRender(renderable *renderables, uint32_t renderablesCount);

void denymWaitForNextFrame(void);

geometryParams geometryCreateParameters(uint32_t vertexCount, uint32_t indexCount);

int geometryParamsAddIndices16(geometryParams params, uint16_t *indices);

int geometryParamsAddIndices32(geometryParams params, uint32_t *indices);

int geometryParamsAddAttribVec2(geometryParams params, float *positions);

int geometryParamsAddAttribVec3(geometryParams params, float *positions);

#define geometryParamsAddPositions2D geometryParamsAddAttribVec2

#define geometryParamsAddPositions3D geometryParamsAddAttribVec3

#define geometryParamsAddColorsRGB geometryParamsAddAttribVec3

#define geometryParamsAddTexCoords geometryParamsAddAttribVec2

#define geometryParamsAddNormals geometryParamsAddAttribVec3

geometry geometryCreate(const geometryParams params);

void geometryDestroy(geometry geometry);

renderable denymCreateRenderable(const renderableCreateParams *createParams);

void denymDestroyRenderable(renderable renderable);

int updateUniformsBuffer(renderable renderable, const void *data);

int updatePushConstants(renderable renderable, float alpha);

float getUptime(void);

renderable modelLoad(const char *objFile, renderableCreateParams *renderableParams, int useIndices, int useNormals);


#endif
