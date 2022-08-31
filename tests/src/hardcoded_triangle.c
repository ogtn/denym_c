#include "denym.h"
#include <stdlib.h>


int main(void)
{
	const int width = 640;
	const int height = 480;
	int result = EXIT_FAILURE;

	if (!denymInit(width, height))
	{
		geometry geometry = denymCreateGeometry(3);
		renderable renderable = denymCreateRenderable(geometry, "hardcoded_triangle.vert.spv", "basic_color_interp.frag.spv");

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
