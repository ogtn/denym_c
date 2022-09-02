#include "denym.h"
#include <stdlib.h>


int main(void)
{
	const int width = 640;
	const int height = 640;

	// clip coordinates
	float positions[] = 
	{
		-0.5f, 0.5f,
		-0.5f, -0.5f,
		0.5f, -0.5f,
        0.5f, 0.5f
	};

	float colors[] =
	{
		0, 1, 0,
		1, 0, 0,
		0, 0, 1,
		1, 1, 1
	};

    uint16_t indices[] = 
    {
        0, 1, 2,
        2, 3, 0
    };

	if (denymInit(width, height))
		return EXIT_FAILURE;

	geometry geometry = denymCreateGeometry(6);
	denymGeometryAddPosition(geometry, positions);
	denymGeometryAddColors(geometry, colors);
    denymGeometryAddIndices(geometry, indices);
	
	renderable square = denymCreateRenderable(
		geometry,
		"basic_position_color_attribute.vert.spv",
		"basic_color_interp.frag.spv");

	while (denymKeepRunning())
	{
		denymRender(square);
		denymWaitForNextFrame();
	}
	
	denymDestroyRenderable(square);
	denymTerminate();

	return EXIT_SUCCESS;
}
