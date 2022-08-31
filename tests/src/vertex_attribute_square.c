#include "denym.h"
#include <stdlib.h>


int main(void)
{
	const int width = 640;
	const int height = 640;
	int result = EXIT_FAILURE;

	// clip coordinates
	float positions[] = 
	{
		-0.5f, 0.5f,
		-0.5f, -0.5f,
		0.5f, -0.5f,

		0.5f, -0.5f,
        0.5f, 0.5f,
		-0.5f, 0.5f
	};

	float colors[] =
	{
		0, 1, 0,
		1, 0, 0,
		0, 0, 1,

        0, 0, 1,
		1, 1, 1,
		0, 1, 0
	};

	if (!denymInit(width, height))
	{
		geometry geometry = denymCreateGeometry(6);
		denymGeometryAddPosition(geometry, positions);
		denymGeometryAddColors(geometry, colors);
		renderable renderable = denymCreateRenderable(geometry, "basic_position_color_attribute.vert.spv", "basic_color_interp.frag.spv");

		result = EXIT_SUCCESS;

		while (denymKeepRunning())
		{
			denymRender();
			denymWaitForNextFrame();
		}
	}

	denymTerminate();

	return result;
}
