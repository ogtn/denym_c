#include "geometry.h"


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
