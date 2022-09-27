#include "denym.h"
#include <stdlib.h>


int main(void)
{
	const int width = 640;
	const int height = 480;

	if (denymInit(width, height))
		return EXIT_FAILURE;

	geometryParams geometryParams = geometryCreateParameters(3, 0);
	renderableCreateParams renderableParams = {
		.geometry = geometryCreate2(geometryParams),
		.vertShaderName = "hardcoded_triangle.vert.spv",
		.fragShaderName = "basic_color_interp.frag.spv"
	};
	renderable triangle = denymCreateRenderable(&renderableParams);

	while (denymKeepRunning())
	{
		denymRender(&triangle, 1);
		denymWaitForNextFrame();
	}

	denymDestroyRenderable(triangle);
	denymTerminate();

	return EXIT_SUCCESS;
}
