#include "geometry.h"
#include "buffer.h"
#include "core.h"

#include <string.h>


geometry geometryCreate(const geometryCreateInfo *createInfo)
{
	geometry geometry = calloc(1, sizeof(*geometry));

	geometry->vertexCount = createInfo->vertexCount;
	geometry->indexCount = createInfo->indexCount;

	if(!geometryAddPositions(geometry, createInfo) &&
		!geometryAddColors(geometry, createInfo) &&
		!geometryAddTexCoods(geometry, createInfo) &&
		!geometryAddIndices(geometry, createInfo))
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
	if(geometry->usePositions)
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

	if(geometry->useIndices)
	{
		vkFreeMemory(engine.vulkanContext.device, geometry->memoryIndices, NULL);
		vkDestroyBuffer(engine.vulkanContext.device, geometry->bufferIndices, NULL);
	}

	free(geometry->vertexAttributeDescriptions);
	free(geometry->vertexBindingDescriptions);
	free(geometry);
}


int geometryAddPositions(geometry geometry, const geometryCreateInfo *createInfo)
{
	if(createInfo->positions)
	{
		geometry->attribCount++;
		geometry->usePositions = VK_TRUE;

		return createVertexBufferWithStaging(
			sizeof(float) * geometry->vertexCount * 2,
			&geometry->bufferPositions,
			&geometry->memoryPositions,
			createInfo->positions);
	}

	return 0;
}


int geometryAddColors(geometry geometry, const geometryCreateInfo *createInfo)
{
	if(createInfo->colors)
	{
		geometry->attribCount++;
		geometry->useColors = VK_TRUE;

		return createVertexBufferWithStaging(
			sizeof(float) * geometry->vertexCount * 3,
			&geometry->bufferColors,
			&geometry->memoryColors,
			createInfo->colors);
	}

	return 0;
}


int geometryAddTexCoods(geometry geometry, const geometryCreateInfo *createInfo)
{
	if(createInfo->texCoords)
	{
		geometry->attribCount++;
		geometry->useTexCoords = VK_TRUE;

		return createVertexBufferWithStaging(
			sizeof(float) * geometry->vertexCount * 2,
			&geometry->bufferTexCoords,
			&geometry->memoryTexCoords,
			createInfo->texCoords);
	}

	return 0;
}

int geometryAddIndices(geometry geometry, const geometryCreateInfo *createInfo)
{
	if((createInfo->indexCount != 0 && createInfo->indices == NULL) ||
	(createInfo->indexCount == 0 && createInfo->indices != NULL))
	{
		fprintf(stderr, "geometryAddIndices() failed (indexCount=%i)(indices=%p)\n",
			createInfo->indexCount, (void*)createInfo->indices);

		return -1;
	}

	if(createInfo->indices)
	{
		geometry->attribCount++;
		geometry->useIndices = VK_TRUE;

		return createIndexBufferWithStaging(
			sizeof(uint16_t) * geometry->indexCount,
			&geometry->bufferIndices,
			&geometry->memoryIndices,
			createInfo->indices);
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

	if(geometry->usePositions) // vec2
	{
		addVertexDescription(geometry, currentBinding, VK_FORMAT_R32G32_SFLOAT, sizeof(float) * 2);
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

	if(geometry->useIndices) // uint16_t
	{
		addVertexDescription(geometry, currentBinding, VK_FORMAT_R16_UINT, sizeof(uint16_t));
		currentBinding++;
	}
}
