#ifndef _geometry_h_
#define _geometry_h_


#include "denym_common.h"


#define GEOMETRY_MAX_ATTRIBS 3


typedef struct geometryParams_t
{
	uint32_t vertexCount;
	uint32_t indexCount;
	uint32_t attribCount;
	void *indices;
	VkIndexType indexType;
	VkDeviceSize indexSize;
	void *data[GEOMETRY_MAX_ATTRIBS];
	VkFormat formats[GEOMETRY_MAX_ATTRIBS];
	uint32_t strides[GEOMETRY_MAX_ATTRIBS];
} geometryParams_t;


typedef struct geometry_t
{
	uint32_t vertexCount;
	uint32_t indexCount;
	uint32_t attribCount;

	VkBuffer buffers[GEOMETRY_MAX_ATTRIBS + 1];
	VkDeviceMemory bufferMemories[GEOMETRY_MAX_ATTRIBS + 1];
	VkBool32 useV2;
	VkIndexType indexType;

	VkBool32 usePositions2D;
	VkBool32 usePositions3D;
	VkBool32 useColors;
	VkBool32 useTexCoords;
	VkBool32 useIndices_16;
	VkBool32 useIndices_32;

	VkBuffer bufferPositions;
	VkBuffer bufferColors;
	VkBuffer bufferTexCoords;
	VkBuffer bufferIndices;

	VkDeviceMemory memoryPositions;
	VkDeviceMemory memoryColors;
	VkDeviceMemory memoryTexCoords;
	VkDeviceMemory memoryIndices;

	VkVertexInputAttributeDescription vertexAttributeDescriptions[GEOMETRY_MAX_ATTRIBS];
	VkVertexInputBindingDescription vertexBindingDescriptions[GEOMETRY_MAX_ATTRIBS];
} geometry_t;


geometryParams geometryCreateParameters(uint32_t vertexCount, uint32_t indexCount);

int geometryParamsAddAttribute(geometryParams params, uint32_t index, void *data, VkFormat format, uint32_t elementSize, uint32_t elementCount);

int geometryParamsAddIndices16(geometryParams params, uint16_t *indices);

int geometryParamsAddIndices32(geometryParams params, uint32_t *indices);

int geometryParamsAddPositions2D(geometryParams params, float *positions);

int geometryParamsAddPositions3D(geometryParams params, float *positions);

int geometryParamsAddColorsRGB(geometryParams params, float *colors);

int geometryParamsAddTexCoords(geometryParams params, float *texCoords);

geometry geometryCreate(const geometryCreateParams *params);

geometry geometryCreate2(const geometryParams params);

void geometryDestroy(geometry geometry);

int geometryAddPositions(geometry geometry, const geometryCreateParams *params);

int geometryAddColors(geometry geometry, const geometryCreateParams *params);

int geometryAddTexCoods(geometry geometry, const geometryCreateParams *params);

int geometryAddIndices(geometry geometry, const geometryCreateParams *params);

void addVertexDescription(geometry geometry, uint32_t binding, VkFormat format, uint32_t stride);

void geometryFillPipelineVertexInputStateCreateInfo(geometry geometry);


#endif
