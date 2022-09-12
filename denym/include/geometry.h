#ifndef _geometry_h_
#define _geometry_h_


#include "denym_common.h"


typedef struct geometry_t
{
	uint32_t vertexCount;
	uint16_t indexCount;
	uint16_t attribCount;

	VkBool32 usePositions;
	VkBool32 useColors;
	VkBool32 useTexCoords;
	VkBool32 useIndices;

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

int geometryAddPositions(geometry geometry, const geometryCreateInfo *createInfo);

int geometryAddColors(geometry geometry, const geometryCreateInfo *createInfo);

int geometryAddTexCoods(geometry geometry, const geometryCreateInfo *createInfo);

int geometryAddIndices(geometry geometry, const geometryCreateInfo *createInfo);

void addVertexDescription(geometry geometry, uint32_t binding, VkFormat format, uint32_t stride);

void geometryFillPipelineVertexInputStateCreateInfo(geometry geometry);


#endif
