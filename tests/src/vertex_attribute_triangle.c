#include "denym.h"
#include <stdlib.h>


int main(void)
{
	const int width = 640;
	const int height = 480;

	// clip coordinates
	float positions[] = 
	{
		0.0f, -0.5f,
		0.5f, 0.5f,
		-0.5f, 0.5f
	};

	float colors[] =
	{
		1, 0, 0,
		0, 1, 0,
		0, 0, 1
	};

	if (denymInit(width, height))
		return EXIT_FAILURE;

	geometryCreateInfo geometryCreateInfo = {
		.vertexCount = 3,
		.positions = positions,
		.colors = colors };

	geometry geometry = geometryCreate(&geometryCreateInfo);
	renderable triangle = denymCreateRenderable(geometry, "basic_position_color_attribute.vert.spv", "basic_color_interp.frag.spv");

	while (denymKeepRunning())
	{
		denymRender(triangle);
		denymWaitForNextFrame();
	}

	denymDestroyRenderable(triangle);
	denymTerminate();

	return EXIT_SUCCESS;
}
