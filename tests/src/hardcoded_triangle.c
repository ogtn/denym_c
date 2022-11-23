#include "denym.h"
#include <stdlib.h>


int main(void)
{
	const int width = 640;
	const int height = 640;

	if (denymInit(width, height))
		return EXIT_FAILURE;

	geometryParams geometryParams = geometryCreateParameters(3, 0);
	renderableCreateParams renderableParams = {
		.geometry = geometryCreate(geometryParams),
		.vertShaderName = "hardcoded_triangle.vert.spv",
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
