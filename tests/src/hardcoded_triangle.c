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

	renderable grid = primitiveCreateGrid(2, 3);
	mat4 matrix;
	glm_mat4_identity(matrix);
	glm_rotate_x(matrix, glm_rad(90), matrix);
	glm_translate_y(matrix, 0.5);
	renderableSetMatrix(grid, matrix);

	while (denymKeepRunning())
	{
		denymRender();
		denymWaitForNextFrame();
	}

	denymTerminate();

	return EXIT_SUCCESS;
}
