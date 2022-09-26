#include "geometry.h"
#include "buffer.h"
#include "core.h"

#include <string.h>


geometry geometryCreate(const geometryCreateParams *params)
{
	geometry geometry = calloc(1, sizeof(*geometry));

	geometry->vertexCount = params->vertexCount;
	geometry->indexCount = params->indexCount;

	if(!geometryAddPositions(geometry, params) &&
		!geometryAddColors(geometry, params) &&
		!geometryAddTexCoods(geometry, params) &&
		!geometryAddIndices(geometry, params))
	{
		geometryFillPipelineVertexInputStateCreateInfo(geometry);
		return geometry;

	}

	fprintf(stderr, "geometryCreateBuffers() failed to create buffers\n");
	geometryDestroy(geometry);
	return NULL;
}


void geometryDestroy(geometry geometry)
{
	if(geometry->usePositions2D || geometry->usePositions3D)
	{
		vkFreeMemory(engine.vulkanContext.device, geometry->memoryPositions, NULL);
		vkDestroyBuffer(engine.vulkanContext.device, geometry->bufferPositions, NULL);
	}

	if(geometry->useColors)
	{
		vkFreeMemory(engine.vulkanContext.device, geometry->memoryColors, NULL);
		vkDestroyBuffer(engine.vulkanContext.device, geometry->bufferColors, NULL);
	}

	if(geometry->useTexCoords)
	{
		vkFreeMemory(engine.vulkanContext.device, geometry->memoryTexCoords, NULL);
		vkDestroyBuffer(engine.vulkanContext.device, geometry->bufferTexCoords, NULL);
	}

	if(geometry->useIndices_16 || geometry->useIndices_32)
	{
		vkFreeMemory(engine.vulkanContext.device, geometry->memoryIndices, NULL);
		vkDestroyBuffer(engine.vulkanContext.device, geometry->bufferIndices, NULL);
	}

	free(geometry->vertexAttributeDescriptions);
	free(geometry->vertexBindingDescriptions);
	free(geometry);
}


int geometryAddPositions(geometry geometry, const geometryCreateParams *params)
{
	if(params->positions2D && params->positions3D)
	{
		fprintf(stderr, "geometryAddPositions() failed: cant' use both 2D and 3D\n");

		return -1;
	}

	float *positions = NULL;
	uint32_t size = sizeof(float) * geometry->vertexCount;

	if(params->positions2D)
	{
		size *= 2;
		geometry->usePositions2D = VK_TRUE;
		positions = params->positions2D;
	}
	else if(params->positions3D)
	{
		size *= 3;
		geometry->usePositions3D = VK_TRUE;
		positions = params->positions3D;
	}

	if(positions)
	{
		geometry->attribCount++;

		return createVertexBufferWithStaging(size, &geometry->bufferPositions, &geometry->memoryPositions, positions);
	}

	return 0;
}


int geometryAddColors(geometry geometry, const geometryCreateParams *params)
{
	if(params->colors)
	{
		geometry->attribCount++;
		geometry->useColors = VK_TRUE;

		return createVertexBufferWithStaging(
			sizeof(float) * geometry->vertexCount * 3,
			&geometry->bufferColors,
			&geometry->memoryColors,
			params->colors);
	}

	return 0;
}


int geometryAddTexCoods(geometry geometry, const geometryCreateParams *params)
{
	if(params->texCoords)
	{
		geometry->attribCount++;
		geometry->useTexCoords = VK_TRUE;

		return createVertexBufferWithStaging(
			sizeof(float) * geometry->vertexCount * 2,
			&geometry->bufferTexCoords,
			&geometry->memoryTexCoords,
			params->texCoords);
	}

	return 0;
}


int geometryAddIndices(geometry geometry, const geometryCreateParams *params)
{
	if((params->indexCount != 0 && params->indices_16 == NULL && params->indices_32 == NULL) ||
		(params->indexCount == 0 && (params->indices_16 != NULL && params->indices_32 != NULL)))
	{
		fprintf(stderr, "geometryAddIndices() failed (indexCount=%i)(indices_16=%p)(indices_32=%p)\n",
			params->indexCount, (void*)params->indices_16, (void*)params->indices_16);

		return -1;
	}

	if(params->indices_16)
	{
		geometry->attribCount++;
		geometry->useIndices_16 = VK_TRUE;

		return createIndexBufferWithStaging(
			sizeof(uint16_t) * geometry->indexCount,
			&geometry->bufferIndices,
			&geometry->memoryIndices,
			params->indices_16);
	}
	else if(params->indices_32)
	{
		geometry->attribCount++;
		geometry->useIndices_32 = VK_TRUE;

		return createIndexBufferWithStaging(
			sizeof(uint32_t) * geometry->indexCount,
			&geometry->bufferIndices,
			&geometry->memoryIndices,
			params->indices_32);
	}

	return 0;
}


void addVertexDescription(geometry geometry, uint32_t binding, VkFormat format, uint32_t stride)
{
	geometry->vertexAttributeDescriptions[binding].binding = binding;
	geometry->vertexAttributeDescriptions[binding].format = format;
	geometry->vertexAttributeDescriptions[binding].location = binding;
	geometry->vertexAttributeDescriptions[binding].offset = 0; // because only one type of data in this array (indices), no interleaving

	geometry->vertexBindingDescriptions[binding].binding = binding;
	geometry->vertexBindingDescriptions[binding].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	geometry->vertexBindingDescriptions[binding].stride = stride;
}


void geometryFillPipelineVertexInputStateCreateInfo(geometry geometry)
{
	geometry->vertexAttributeDescriptions = malloc(sizeof * geometry->vertexAttributeDescriptions * geometry->attribCount);
	geometry->vertexBindingDescriptions = malloc(sizeof * geometry->vertexBindingDescriptions * geometry->attribCount);

	// TODO: here we could have had all attributes in the same array
	// in this case, only one vertexBindingDescriptions, and one vertextAttributeDescription per attribute

	uint32_t currentBinding = 0;

	if(geometry->usePositions2D) // vec2
	{
		addVertexDescription(geometry, currentBinding, VK_FORMAT_R32G32_SFLOAT, sizeof(float) * 2);
		currentBinding++;
	}

	if(geometry->usePositions3D) // vec3
	{
		addVertexDescription(geometry, currentBinding, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 3);
		currentBinding++;
	}

	if(geometry->useColors) // vec3
	{
		addVertexDescription(geometry, currentBinding, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 3);
		currentBinding++;
	}

	if(geometry->useTexCoords) // vec2
	{
		addVertexDescription(geometry, currentBinding, VK_FORMAT_R32G32_SFLOAT, sizeof(float) * 2);
		currentBinding++;
	}

	if(geometry->useIndices_16)
	{
		addVertexDescription(geometry, currentBinding, VK_FORMAT_R16_UINT, sizeof(uint16_t));
		currentBinding++;
	}

	if(geometry->useIndices_32)
	{
		addVertexDescription(geometry, currentBinding, VK_FORMAT_R32_UINT, sizeof(uint32_t));
		currentBinding++;
	}
}
