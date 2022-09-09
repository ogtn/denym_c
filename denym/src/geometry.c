#include "geometry.h"
#include "buffer.h"
#include "core.h"

#include <string.h>


geometry geometryCreate(const geometryCreateInfo *createInfo)
{
	if((createInfo->indexCount != 0 && createInfo->indices == NULL) ||
		(createInfo->indexCount == 0 && createInfo->indices != NULL))
	{
		fprintf(stderr, "geometryCreate() failed (indexCount=%i)(indices=%p)\n",
			createInfo->indexCount, (void*)createInfo->indices);

		return NULL;
	}

	geometry geometry = calloc(1, sizeof(*geometry));

	geometry->vertexCount = createInfo->vertexCount;
	geometry->indexCount = createInfo->indexCount;
	geometry->attribCount =
		!!createInfo->colors +
		!!createInfo->positions +
		!!createInfo->texCoords +
		!!createInfo->indices;

	geometry->positions = createInfo->positions;
	geometry->colors = createInfo->colors;
	geometry->texCoords = createInfo->texCoords;
	geometry->indices = createInfo->indices;

	if(geometryCreateBuffers(geometry))
	{
		free(geometry);
		fprintf(stderr, "geometryCreateBuffers() failed to create buffers\n");

		return NULL;
	}

	geometryFillPipelineVertexInputStateCreateInfo(geometry);

	return geometry;
}


void geometryDestroy(geometry geometry)
{
	if(geometry->positions)
	{
		vkFreeMemory(engine.vulkanContext.device, geometry->memoryPositions, NULL);
		vkDestroyBuffer(engine.vulkanContext.device, geometry->bufferPositions, NULL);
	}

	if(geometry->colors)
	{
		vkFreeMemory(engine.vulkanContext.device, geometry->memoryColors, NULL);
		vkDestroyBuffer(engine.vulkanContext.device, geometry->bufferColors, NULL);
	}

	if(geometry->texCoords)
	{
		vkFreeMemory(engine.vulkanContext.device, geometry->memoryTexCoords, NULL);
		vkDestroyBuffer(engine.vulkanContext.device, geometry->bufferTexCoords, NULL);
	}

	if(geometry->indices)
	{
		vkFreeMemory(engine.vulkanContext.device, geometry->memoryIndices, NULL);
		vkDestroyBuffer(engine.vulkanContext.device, geometry->bufferIndices, NULL);
	}

	free(geometry->vertexAttributeDescriptions);
	free(geometry->vertexBindingDescriptions);
	free(geometry);
}


int geometryCreateBuffers(geometry geometry)
{
	if(geometry->positions)
		createVertexBufferWithStaging(
			sizeof * geometry->positions * geometry->vertexCount * 2,
			&geometry->bufferPositions,
			&geometry->memoryPositions,
			geometry->positions);

	if(geometry->colors)
		createVertexBufferWithStaging(
			sizeof * geometry->colors * geometry->vertexCount * 3,
			&geometry->bufferColors,
			&geometry->memoryColors,
			geometry->colors);

	if(geometry->texCoords)
		createVertexBufferWithStaging(
			sizeof * geometry->texCoords * geometry->vertexCount * 2,
			&geometry->bufferTexCoords,
			&geometry->memoryTexCoords,
			geometry->texCoords);

	if(geometry->indices)
		createIndexBufferWithStaging(
			sizeof * geometry->indices * geometry->indexCount,
			&geometry->bufferIndices,
			&geometry->memoryIndices,
			geometry->indices);

	return 0; // TODO, that sucks pretty bad
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

	if(geometry->positions) // vec2
	{
		addVertexDescription(geometry, currentBinding, VK_FORMAT_R32G32_SFLOAT, sizeof(float) * 2);
		currentBinding++;
	}

	if(geometry->colors) // vec3
	{
		addVertexDescription(geometry, currentBinding, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float) * 3);
		currentBinding++;
	}

	if(geometry->texCoords) // vec2
	{
		addVertexDescription(geometry, currentBinding, VK_FORMAT_R32G32_SFLOAT, sizeof(float) * 2);
		currentBinding++;
	}

	if(geometry->indices) // uint16_t
	{
		addVertexDescription(geometry, currentBinding, VK_FORMAT_R16_UINT, sizeof(uint16_t));
		currentBinding++;
	}
}
