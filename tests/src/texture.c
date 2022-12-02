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
	primitiveCreateGrid(8, 3);

	vec3 eye = {1, 1, 2};
	vec3 center = { 0, 0, 0};

	input_t input;
	camera camera = cameraCreatePerspective(60, 0.01f, 1000.f);
	cameraLookAt(camera, eye, center);
	sceneSetCamera(denymGetScene(), camera);

	while(denymKeepRunning(&input))
	{
        float elapsed_since_start = getUptime();
		mat4 matrix;

		updateCameraPerspective(&input, camera);
	    glm_mat4_identity(matrix);
		glm_translate_z(matrix, -0.5f);
		glm_rotate_z(matrix, -glm_rad(elapsed_since_start * 100), matrix);
        renderableSetMatrix(coloredSquare, matrix);

		glm_mat4_identity(matrix);
		glm_rotate_z(matrix, glm_rad(elapsed_since_start * 50), matrix);
		renderableSetMatrix(texturedSquare, matrix);

		denymRender();
		denymWaitForNextFrame();
	}

	denymTerminate();

	return EXIT_SUCCESS;
}
