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


int geometryParamsAddAttribute(geometryParams params, void *data, VkFormat format, uint32_t elementSize, uint32_t elementCount)
{
	if(!params || !data || params->attribCount == GEOMETRY_MAX_ATTRIBS)
		return -1;

	params->data[params->attribCount] = data;
	params->formats[params->attribCount] = format;
	params->strides[params->attribCount] = elementSize * elementCount;
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
	return geometryParamsAddAttribute(params, positions, VK_FORMAT_R32G32_SFLOAT, sizeof(float), 2);
}


int geometryParamsAddPositions3D(geometryParams params, float *positions)
{
	return geometryParamsAddAttribute(params, positions, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float), 3);
}


int geometryParamsAddColorsRGB(geometryParams params, float *colors)
{
	return geometryParamsAddAttribute(params, colors, VK_FORMAT_R32G32B32_SFLOAT, sizeof(float), 3);
}


int geometryParamsAddTexCoords(geometryParams params, float *texCoords)
{
	return geometryParamsAddAttribute(params, texCoords, VK_FORMAT_R32G32_SFLOAT, sizeof(float), 2);
}


geometry geometryCreate(const geometryParams params)
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
	uint32_t lastBuffer = geometry->attribCount + !!geometry->indexCount;

	for(uint32_t i = 0; i < lastBuffer; i++)
	{
		vkFreeMemory(engine.vulkanContext.device, geometry->bufferMemories[i], NULL);
		vkDestroyBuffer(engine.vulkanContext.device, geometry->buffers[i], NULL);
	}

	free(geometry);
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
