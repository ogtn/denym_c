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

	if (denymInit(width, height))
		return EXIT_FAILURE;

	geometryCreateParams geometryParams = {
		.vertexCount = 6,
		.positions2D = positions,
		.colors = colors };

	renderableCreateParams renderableParams = {
		.geometry = geometryCreate(&geometryParams),
		.vertShaderName = "basic_position_color_attribute.vert.spv",
		.fragShaderName = "basic_color_interp.frag.spv"
	};
	renderable square = denymCreateRenderable(&renderableParams);

	while (denymKeepRunning())
	{
		denymRender(&square, 1);
		denymWaitForNextFrame();
	}

	denymDestroyRenderable(square);
	denymTerminate();

	return EXIT_SUCCESS;
}
