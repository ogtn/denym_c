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

	// TODO: here we could have had all attributes and indices in the same array
	VkBuffer buffers[GEOMETRY_MAX_ATTRIBS + 1];
	VkDeviceMemory bufferMemories[GEOMETRY_MAX_ATTRIBS + 1];
	VkIndexType indexType;

	VkVertexInputAttributeDescription vertexAttributeDescriptions[GEOMETRY_MAX_ATTRIBS];
	VkVertexInputBindingDescription vertexBindingDescriptions[GEOMETRY_MAX_ATTRIBS];
} geometry_t;


geometryParams geometryCreateParameters(uint32_t vertexCount, uint32_t indexCount);

int geometryParamsAddAttribute(geometryParams params, void *data, VkFormat format, uint32_t elementSize, uint32_t elementCount);

int geometryParamsAddIndices16(geometryParams params, uint16_t *indices);

int geometryParamsAddIndices32(geometryParams params, uint32_t *indices);

int geometryParamsAddAttribVec2(geometryParams params, float *positions);

int geometryParamsAddAttribVec3(geometryParams params, float *positions);

geometry geometryCreate(const geometryParams params);

void geometryDestroy(geometry geometry);

void geometryAddVertexDescription(geometry geometry, uint32_t binding, VkFormat format, uint32_t stride);

void geometryFillVertexInputInfo(geometry geometry, VkPipelineVertexInputStateCreateInfo *vertexInputInfo);

void geometryDraw(geometry geometry, VkCommandBuffer commandBuffer, uint32_t instanceCount);

#endif
