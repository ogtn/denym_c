#include "geometry.h"
#include "buffer.h"
#include "core.h"


static geometry_t static_geometry;


geometry denymCreateGeometry(uint32_t vertexCount)
{
	geometry geometry = &static_geometry;
	geometry->vertexCount = vertexCount;

	geometry->attribCount = 0;
	geometry->colors =  NULL;
	geometry->positions = NULL;

	return geometry;
}


void denymGeometryAddPosition(geometry geometry, float *positions)
{
	geometry->attribCount++;
	geometry->positions = positions;
}


void denymGeometryAddColors(geometry geometry, float *colors)
{
	geometry->attribCount++;
	geometry->colors = colors;
}


void denymGeometryAddIndices(geometry geometry, uint16_t *indices)
{
	// TODO indicesCount should be equal to vertexCount
	geometry->attribCount++;
	geometry->indices = indices;
}


void denymDestroyGeometry(geometry geometry)
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


int createBuffers(geometry geometry)
{
	if(geometry->positions)
		createVertexBufferWithStaging(
			sizeof * geometry->positions * geometry->vertexCount * 2, // TODO vertexCount is wrong here... should be posCount
			&geometry->bufferPositions,
			&geometry->memoryPositions,
			geometry->positions);

	if(geometry->colors)
		createVertexBufferWithStaging(
			sizeof * geometry->colors * geometry->vertexCount * 3, // TODO same here
			&geometry->bufferColors,
			&geometry->memoryColors,
			geometry->colors);

	if(geometry->indices)
		createIndexBufferWithStaging(
			sizeof * geometry->indices * geometry->vertexCount, // TODO this is ok though, vertexCount should be equal to indicesCount, but we need to check this when adding them
			&geometry->bufferIndices,
			&geometry->memoryIndices,
			geometry->indices);

	return 0; // TODO, that sucks pretty bad
}
