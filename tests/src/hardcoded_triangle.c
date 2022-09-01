#include "denym.h"
#include <stdlib.h>


int main(void)
{
	const int width = 640;
	const int height = 480;

	if (denymInit(width, height))
		return EXIT_FAILURE;
	
	geometry geometry = denymCreateGeometry(3);
	renderable triangle = denymCreateRenderable(
		geometry,
		"hardcoded_triangle.vert.spv",
		"basic_color_interp.frag.spv");

	while (denymKeepRunning())
	{
		denymRender(triangle);
		denymWaitForNextFrame();
	}

	denymDestroyRenderable(triangle);
	denymTerminate();

	return EXIT_SUCCESS;
}
