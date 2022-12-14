#include "camera_update.h"

#include <stdlib.h>
#include <math.h>


static renderable makeSquare(const char *vertShader, const char *fragShader)
{
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

    float texCoords[] =
	{
		0, 1,
		0, 0,
		1, 0,
		1, 1,
	};

    uint16_t indices[] =
    {
        0, 1, 2,
        2, 3, 0
    };

	geometryParams geometryParams = geometryCreateParameters(4, sizeof indices / sizeof *indices);
	geometryParamsAddPositions2D(geometryParams, positions);
	geometryParamsAddColorsRGB(geometryParams, colors);
	geometryParamsAddTexCoords(geometryParams, texCoords);
	geometryParamsAddIndices16(geometryParams, indices);

	renderableCreateParams renderableParams = {
		.vertShaderName = vertShader,
		.fragShaderName = fragShader,
		.geometry = geometryCreate(geometryParams),
		.textureName = "lena.jpg",
		.sendMVP = 1
	};

    return renderableCreate(&renderableParams, 1);
}


int main(void)
{
	const int width = 640;
	const int height = 480;

	if (denymInit(width, height))
		return EXIT_FAILURE;

    renderable coloredSquare = makeSquare("mvp_ubo_position_color_attribute.vert.spv", "basic_color_interp.frag.spv");
    renderable texturedSquare = makeSquare("texture.vert.spv", "texture.frag.spv");
	renderableSetPosition(coloredSquare, 0, 0, -0.5f);

	vec3 eye = {1, 1, 2};
	vec3 center = { 0, 0, 0};

	input_t input;
	camera camera = cameraCreatePerspective(60, 0.01f, 1000.f);
	cameraLookAt(camera, eye, center);
	sceneSetCamera(denymGetScene(), camera);
	primitiveCreateGrid(8, 3);

	while(denymKeepRunning(&input))
	{
        float angularSpeed = denymGetTimeSinceLastFrame() * 20;

		renderableRotateZ(coloredSquare, angularSpeed);
		renderableRotateZ(texturedSquare, angularSpeed);

		updateCameraPerspective(&input, camera);
		denymRender();
		denymWaitForNextFrame();
	}

	denymTerminate();

	return EXIT_SUCCESS;
}
