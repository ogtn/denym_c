#ifndef _geometry_h_
#define _geometry_h_


#include "denym_common.h"


typedef struct geometry_t
{
	uint32_t vertexCount;
	uint16_t indiceCount;
	uint16_t attribCount;

	float *positions;
	float *colors;
	uint16_t *indices;

	VkBuffer bufferPositions;
	VkBuffer bufferColors;
	VkBuffer bufferIndices;

	VkDeviceMemory memoryPositions;
	VkDeviceMemory memoryColors;
	VkDeviceMemory memoryIndices;
} geometry_t;


geometry geometryCreate(const geometryCreateInfo *createInfo);

void geometryDestroy(geometry geometry);

int geometryCreateBuffers(geometry geometry);


#endif
