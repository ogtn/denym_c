#include "denym.h"
#include <stdlib.h>
#include <math.h>


static renderable makeSquare(void)
{
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
        0, 1, 2,
        2, 3, 0
    };

	geometryParams geometryParams = geometryCreateParameters(4, sizeof indices / sizeof *indices);
	geometryParamsAddPositions2D(geometryParams, positions);
	geometryParamsAddColorsRGB(geometryParams, colors);
	geometryParamsAddIndices16(geometryParams, indices);

	renderableCreateParams renderableParams = {
		.geometry = geometryCreate(geometryParams),
		.vertShaderName = "mvp_ubo_position_color_attribute.vert.spv",
		.fragShaderName = "basic_color_interp.frag.spv",
		.uniformSize = sizeof(modelViewProj)
	};

    return denymCreateRenderable(&renderableParams);
}


int main(void)
{
	const int width = 640;
	const int height = 480;

	if (denymInit(width, height))
		return EXIT_FAILURE;

    renderable square1 = makeSquare();
    renderable square = makeSquare();

	vec3 eye = {2, 2, 2};
	vec3 center = { 0, 0, 0};

	camera camera = cameraCreatePerspective(45, (float)width / (float)height, 0.01f, 1000.f);
	cameraLookAt(camera, eye, center);
	sceneSetCamera(denymGetScene(), camera);

	while (denymKeepRunning())
	{
		float elapsed_since_start = getUptime();
		mat4 matrix;

		glm_mat4_identity(matrix);
		glm_translate_z(matrix, -0.5f);
		glm_rotate_z(matrix, -glm_rad(elapsed_since_start * 100), matrix);
		renderableSetMatrix(square1, matrix);

		glm_mat4_identity(matrix);
		glm_rotate_z(matrix, glm_rad(elapsed_since_start * 100), matrix);
		renderableSetMatrix(square, matrix);

		denymRender();
		denymWaitForNextFrame();
	}

	denymTerminate();

	return EXIT_SUCCESS;
}
