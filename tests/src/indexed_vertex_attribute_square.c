#include "camera_update.h"

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
        0, 2, 1,
        2, 0, 3
    };

	if (denymInit(width, height))
		return EXIT_FAILURE;

	geometryParams geometryParams = geometryCreateParameters(4, sizeof indices / sizeof *indices);
	geometryParamsAddPositions2D(geometryParams, positions);
	geometryParamsAddColorsRGB(geometryParams, colors);
	geometryParamsAddIndices16(geometryParams, indices);

	renderableCreateParams renderableParams = {
		.geometry = geometryCreate(geometryParams),
		.vertShaderName = "basic_position_color_attribute.vert.spv",
		.fragShaderName = "basic_color_interp.frag.spv"
	};
	renderableCreate(&renderableParams);
	primitiveCreateGrid(2, 3);

	while(denymKeepRunning(NULL))
	{
		denymRender();
		denymWaitForNextFrame();
	}

	denymTerminate();

	return EXIT_SUCCESS;
}
