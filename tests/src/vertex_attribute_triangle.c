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

	geometryParams geometryParams = geometryCreateParameters(3, 0);
	geometryParamsAddPositions2D(geometryParams, positions);
	geometryParamsAddColorsRGB(geometryParams, colors);

	renderableCreateParams renderableParams = {
		.geometry = geometryCreate2(geometryParams),
		.vertShaderName = "basic_position_color_attribute.vert.spv",
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
