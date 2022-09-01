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
	VkBuffer bufferPositions;
	VkBuffer bufferColors;
	VkDeviceMemory memoryPositions;
	VkDeviceMemory memoryColors;

	// constants

	// uniforms
} geometry_t;


int createVertexBuffers(geometry geometry);


#endif
