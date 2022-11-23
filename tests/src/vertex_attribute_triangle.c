#include "denym.h"
#include <stdlib.h>


int main(void)
{
	const int width = 640;
	const int height = 640;

	// clip coordinates
	float positions[] =
	{
		0.5f, 0.5f,
		0.0f, -0.5f,
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
		.geometry = geometryCreate(geometryParams),
		.vertShaderName = "basic_position_color_attribute.vert.spv",
		.fragShaderName = "basic_color_interp.frag.spv"
	};
	renderableCreate(&renderableParams);
	primitiveCreateGrid(2, 3);

	while (denymKeepRunning())
	{
		denymRender();
		denymWaitForNextFrame();
	}

	denymTerminate();

	return EXIT_SUCCESS;
}
