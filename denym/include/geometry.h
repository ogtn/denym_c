#ifndef _geometry_h_
#define _geometry_h_


#include "denym_common.h"


typedef struct geometry_t
{
	uint32_t vertexCount;

	// attributes
	uint32_t attribCount;
	float *positions;
	float *colors;
	uint16_t *indices;
	VkBuffer bufferPositions;
	VkBuffer bufferColors;
	VkBuffer bufferIndices;
	VkDeviceMemory memoryPositions;
	VkDeviceMemory memoryColors;
	VkDeviceMemory memoryIndices;

	// constants

	// uniforms
} geometry_t;


int createBuffers(geometry geometry);


#endif
