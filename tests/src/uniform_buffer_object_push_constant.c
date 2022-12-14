#include "camera_update.h"

#include <stdlib.h>
#include <math.h>


int main(void)
{
	const int width = 640;
	const int height = 480;

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

	if (denymInit(width, height))
		return EXIT_FAILURE;

	geometryParams geometryParams = geometryCreateParameters(4, sizeof indices / sizeof *indices);
	geometryParamsAddPositions2D(geometryParams, positions);
	geometryParamsAddColorsRGB(geometryParams, colors);
	geometryParamsAddIndices16(geometryParams, indices);

	renderableCreateParams renderableParams = {
		.geometry = geometryCreate(geometryParams),
		.vertShaderName = "mvp_ubo_position_color_attribute.vert.spv",
		.fragShaderName = "basic_color_with_alpha_cst.frag.spv",
		.sendMVP = 1,
		.pushConstantSize = sizeof(float)
	};

	renderable square = renderableCreate(&renderableParams, 1);
	renderableSetPosition(square, 0, 0, 0.1f);

	vec3 eye = {2, 2, 2};
	vec3 center = { 0, 0, 0};

	input_t input;
	camera camera = cameraCreatePerspective(60, 0.01f, 1000.f);
	cameraLookAt(camera, eye, center);
	sceneSetCamera(denymGetScene(), camera);
	primitiveCreateGrid(8, 3);

	while(denymKeepRunning(&input))
	{
        float elapsed_since_start = getUptime();
        float alpha = (sinf(elapsed_since_start * 4) + 1) / 2;
		float angularSpeed = denymGetTimeSinceLastFrame() * 20;

	    renderableRotateZ(square, angularSpeed);
        renderableUpdatePushConstant(square, &alpha);

		updateCameraPerspective(&input, camera);
		denymRender();
		denymWaitForNextFrame();
	}

	denymTerminate();

	return EXIT_SUCCESS;
}
