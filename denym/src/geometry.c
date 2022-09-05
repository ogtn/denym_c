#include "geometry.h"
#include "buffer.h"
#include "core.h"

#include <string.h>


geometry geometryCreate(const geometryCreateInfo *createInfo)
{
	if((createInfo->indiceCount != 0 && createInfo->indices == NULL) ||
		(createInfo->indiceCount == 0 && createInfo->indices != NULL))
	{
		fprintf(stderr, "geometryCreate() failed (indiceCount=%i)(indices=%p)\n",
			createInfo->indiceCount, (void*)createInfo->indices);

		return NULL;
	}

	geometry geometry = calloc(1, sizeof(*geometry));

	geometry->vertexCount = createInfo->vertexCount;
	geometry->indiceCount = createInfo->indiceCount;
	geometry->attribCount =
		!!createInfo->colors +
		!!createInfo->positions +
		!!createInfo->indices;

	geometry->positions = createInfo->positions;
	geometry->colors = createInfo->colors;
	geometry->indices = createInfo->indices;

	if(geometryCreateBuffers(geometry))
	{
		free(geometry);
		fprintf(stderr, "geometryCreateBuffers() failed to create buffers\n");

		return NULL;
	}

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

	if(geometry->indices)
	{
		vkFreeMemory(engine.vulkanContext.device, geometry->memoryIndices, NULL);
		vkDestroyBuffer(engine.vulkanContext.device, geometry->bufferIndices, NULL);
	}

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

	if(geometry->indices)
		createIndexBufferWithStaging(
			sizeof * geometry->indices * geometry->indiceCount,
			&geometry->bufferIndices,
			&geometry->memoryIndices,
			geometry->indices);

	return 0; // TODO, that sucks pretty bad
}


void geometryFillPipelineVertexInputStateCreateInfo(geometry geometry, VkPipelineVertexInputStateCreateInfo *createInfo)
{
	memset(createInfo, 0, sizeof *createInfo);
	createInfo->sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

	if(geometry->attribCount == 0)
		return;

	createInfo->vertexBindingDescriptionCount = geometry->attribCount;
	createInfo->vertexAttributeDescriptionCount = geometry->attribCount;

	// TODO: free those at some point
	VkVertexInputAttributeDescription *vertextAttributeDescriptions = malloc(sizeof * vertextAttributeDescriptions * geometry->attribCount);
	VkVertexInputBindingDescription *vertexBindingDescriptions = malloc(sizeof * vertexBindingDescriptions * geometry->attribCount);

	// TODO: here we could have had positions, colors and indices in the same array
	// in this case, only one vertexBindingDescriptions, and two vertextAttributeDescriptions

	if(geometry->positions)
	{
		vertextAttributeDescriptions[0].binding = 0;
		vertextAttributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT; // vec2
		vertextAttributeDescriptions[0].location = 0;
		vertextAttributeDescriptions[0].offset = 0; // because only one type of data in this array (positions), no interleaving

		vertexBindingDescriptions[0].binding = 0;
		vertexBindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		vertexBindingDescriptions[0].stride = sizeof(float) * 2;
	}

	if(geometry->colors)
	{
		vertextAttributeDescriptions[1].binding = 1;
		vertextAttributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT; // vec3
		vertextAttributeDescriptions[1].location = 1;
		vertextAttributeDescriptions[1].offset = 0; // because only one type of data in this array (colors), no interleaving

		vertexBindingDescriptions[1].binding = 1;
		vertexBindingDescriptions[1].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		vertexBindingDescriptions[1].stride = sizeof(float) * 3;
	}

	if(geometry->indices)
	{
		vertextAttributeDescriptions[2].binding = 2;
		vertextAttributeDescriptions[2].format = VK_FORMAT_R16_UINT; // uint16_t
		vertextAttributeDescriptions[2].location = 2;
		vertextAttributeDescriptions[2].offset = 0; // because only one type of data in this array (indices), no interleaving

		vertexBindingDescriptions[2].binding = 2;
		vertexBindingDescriptions[2].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		vertexBindingDescriptions[2].stride = sizeof(uint16_t);
	}

	createInfo->pVertexAttributeDescriptions = vertextAttributeDescriptions;
	createInfo->pVertexBindingDescriptions = vertexBindingDescriptions;
}
