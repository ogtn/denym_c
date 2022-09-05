#include "geometry.h"
#include "buffer.h"
#include "core.h"


static geometry_t static_geometry;


geometry geometryCreate(const geometryCreateInfo *createInfo)
{
	geometry geometry = &static_geometry;

	geometry->vertexCount = createInfo->vertexCount;
	geometry->indiceCount = createInfo->indiceCount;
	geometry->attribCount =
		!!createInfo->colors +
		!!createInfo->positions +
		!!createInfo->indices;

	geometry->positions = createInfo->positions;
	geometry->colors = createInfo->colors;
	geometry->indices = createInfo->indices;

	if((createInfo->indiceCount != 0 && createInfo->indices == NULL) ||
		(createInfo->indiceCount == 0 && createInfo->indices != NULL))
		return NULL;

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
