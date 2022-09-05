#include "denym.h"
#include <stdlib.h>


int main(void)
{
	const int width = 640;
	const int height = 480;

	if (denymInit(width, height))
		return EXIT_FAILURE;
	
	geometryCreateInfo geometryCreateInfo = { .vertexCount = 3 };
	geometry geometry = geometryCreate(&geometryCreateInfo);
	renderable triangle = denymCreateRenderable(
		geometry,
		"hardcoded_triangle.vert.spv",
		"basic_color_interp.frag.spv");

	while (denymKeepRunning())
	{
		denymRender(&triangle, 1);
		denymWaitForNextFrame();
	}

	denymDestroyRenderable(triangle);
	denymTerminate();

	return EXIT_SUCCESS;
}
