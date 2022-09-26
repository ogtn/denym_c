#ifndef _geometry_h_
#define _geometry_h_


#include "denym_common.h"


typedef struct geometry_t
{
	uint32_t vertexCount;
	uint32_t indexCount;
	uint16_t attribCount;

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

	VkVertexInputAttributeDescription *vertexAttributeDescriptions;
	VkVertexInputBindingDescription *vertexBindingDescriptions;
} geometry_t;


geometry geometryCreate(const geometryCreateParams *params);

void geometryDestroy(geometry geometry);

int geometryAddPositions(geometry geometry, const geometryCreateParams *params);

int geometryAddColors(geometry geometry, const geometryCreateParams *params);

int geometryAddTexCoods(geometry geometry, const geometryCreateParams *params);

int geometryAddIndices(geometry geometry, const geometryCreateParams *params);

void addVertexDescription(geometry geometry, uint32_t binding, VkFormat format, uint32_t stride);

void geometryFillPipelineVertexInputStateCreateInfo(geometry geometry);


#endif
