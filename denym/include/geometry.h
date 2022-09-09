#ifndef _geometry_h_
#define _geometry_h_


#include "denym_common.h"


typedef struct geometry_t
{
	uint32_t vertexCount;
	uint16_t indexCount;
	uint16_t attribCount;

	float *positions;
	float *colors;
	float *texCoords;
	uint16_t *indices;

	VkBuffer bufferPositions;
	VkBuffer bufferColors;
	VkBuffer bufferTexCoords;
	VkBuffer bufferIndices;

	VkDeviceMemory memoryPositions;
	VkDeviceMemory memoryColors;
	VkDeviceMemory memoryTexCoords;
	VkDeviceMemory memoryIndices;

	VkVertexInputAttributeDescription *vertexAttributeDescriptions;
	VkVertexInputBindingDescription *vertexBindingDescriptions;
} geometry_t;


geometry geometryCreate(const geometryCreateInfo *createInfo);

void geometryDestroy(geometry geometry);

int geometryCreateBuffers(geometry geometry);

void addVertexDescription(geometry geometry, uint32_t binding, VkFormat format, uint32_t stride);

void geometryFillPipelineVertexInputStateCreateInfo(geometry geometry);


#endif
