#include "geometry.h"
#include "buffer.h"
#include "core.h"

#include <string.h>


geometryParams geometryCreateParameters(uint32_t vertexCount, uint32_t indexCount)
{
	geometryParams params = calloc(1, sizeof *params);

	params->vertexCount = vertexCount;
	params->indexCount = indexCount;

	if(indexCount == 0)
		params->indexType = VK_INDEX_TYPE_NONE_KHR;

	return params;
}


int geometryParamsAddAttribute(geometryParams params, uint32_t index, void *data, VkFormat format, uint32_t elementSize, uint32_t elementCount)
{
	if(!params || !data || params->attribCount == GEOMETRY_MAX_ATTRIBS)
		return -1;

	params->data[index] = data;
	params->formats[index] = format;
	params->strides[index] = elementSize * elementCount;
	params->attribCount++;

	return 0;
}


int geometryParamsAddIndices16(geometryParams params, uint16_t *indices)
{
	if(!params || !indices || !params->indexCount)
		return -1;

	params->indices = indices;
	params->indexType = VK_INDEX_TYPE_UINT16;
	params->indexSize = params->indexCount * sizeof *indices;

	return 0;
}


int geometryParamsAddIndices32(geometryParams params, uint32_t *indices)
{
	if(!params || !indices || !params->indexCount)
		return -1;

	params->indices = indices;
	params->indexType = VK_INDEX_TYPE_UINT32;
	params->indexSize = params->indexCount * sizeof *indices;

	return 0;
}


int geometryParamsAddPositions2D(geometryParams params, float *positions)
{
	return geometryParamsAddAttribute(params, params->attribCount, positions, VK_FORMAT_R32G32_SFLOAT, sizeof(float), 2);
}


int geometryParamsAddPositions3D(geometryParams params, float *positions)
{
	return geometryParamsAddAttribute(params, params->attribCount, positions, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float), 3);
}


int geometryParamsAddColorsRGB(geometryParams params, float *colors)
{
	return geometryParamsAddAttribute(params, params->attribCount, colors, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float), 3);
}


int geometryParamsAddTexCoords(geometryParams params, float *texCoords)
{
	return geometryParamsAddAttribute(params, params->attribCount, texCoords, VK_FORMAT_R32G32_SFLOAT, sizeof(float), 2);
}


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


geometry geometryCreate2(const geometryParams params)
{
	if(params->vertexCount == 0)
	{
		fprintf(stderr, "geometryCreate2() vertexCount can't be 0\n");

		return NULL;
	}

	if(params->indexCount && params->data[0] == NULL)
	{
		fprintf(stderr, "geometryCreate2() indices are missing\n");

		return NULL;
	}

	geometry geometry = calloc(1, sizeof(*geometry));

	geometry->vertexCount = params->vertexCount;
	geometry->indexCount = params->indexCount;
	geometry->indexType = params->indexType;
	geometry->attribCount = params->attribCount;
	geometry->useV2 = VK_TRUE;

	for(uint32_t i = 0; i < geometry->attribCount; i++)
	{
		VkDeviceSize size = params->strides[i] * geometry->vertexCount;

		// TODO: use directly createBufferWithStaging() and delete createVertexBufferWithStaging()
		if(createVertexBufferWithStaging(size, &geometry->buffers[i], &geometry->bufferMemories[i],	params->data[i]))
		{
			fprintf(stderr, "geometryCreate2() failed to create buffers for attribute %d\n", i);
			goto error;
		}

		addVertexDescription(geometry, i, params->formats[i], params->strides[i]);
	}

	if(geometry->indexCount)
	{
		// TODO: use directly createBufferWithStaging() and delete createIndexBufferWithStaging()
		if(createIndexBufferWithStaging(params->indexSize, &geometry->buffers[geometry->attribCount], &geometry->bufferMemories[geometry->attribCount],	params->indices))
		{
			fprintf(stderr, "geometryCreate2() failed to create buffers for indices\n");
			goto error;
		}
	}

	free(params);

	return geometry;

	error:
	geometryDestroy(geometry);

	return NULL;
}


void geometryDestroy(geometry geometry)
{
	if(geometry->useV2)
	{
		uint32_t lastBuffer = geometry->attribCount + !!geometry->indexCount;

		for(uint32_t i = 0; i < lastBuffer; i++)
		{
			vkFreeMemory(engine.vulkanContext.device, geometry->bufferMemories[i], NULL);
			vkDestroyBuffer(engine.vulkanContext.device, geometry->buffers[i], NULL);
		}
	}
	else
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
	}

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
		//geometry->attribCount++;
		geometry->useIndices_16 = VK_TRUE;

		return createIndexBufferWithStaging(
			sizeof(uint16_t) * geometry->indexCount,
			&geometry->bufferIndices,
			&geometry->memoryIndices,
			params->indices_16);
	}
	else if(params->indices_32)
	{
		//geometry->attribCount++;
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
	geometry->vertexAttributeDescriptions[binding].offset = 0; // because only one type of data in this array, no interleaving

	geometry->vertexBindingDescriptions[binding].binding = binding;
	geometry->vertexBindingDescriptions[binding].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
	geometry->vertexBindingDescriptions[binding].stride = stride;
}


void geometryFillPipelineVertexInputStateCreateInfo(geometry geometry)
{
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
	/*
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
	*/
}
