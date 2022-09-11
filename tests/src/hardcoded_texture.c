#include "denym.h"
#include <stdlib.h>
#include <math.h>


static renderable makeSquare(const char *vertShader, const char *fragShader)
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

	geometryCreateInfo geometryCreateInfo = {
		.vertexCount = 4,
		.positions = positions,
		.colors = colors,
        .texCoords = texCoords,
		.indices = indices,
		.indexCount = sizeof indices / sizeof *indices };

	geometry geometry = geometryCreate(&geometryCreateInfo);
	renderable square = denymCreateRenderable(geometry,	vertShader,	fragShader);
    useUniforms(square);

    return square;
}


int main(void)
{
	const int width = 640;
	const int height = 480;

	if (denymInit(width, height))
		return EXIT_FAILURE;

    renderable coloredSquare = makeSquare("mvp_ubo_position_color_attribute.vert.spv", "basic_color_interp.frag.spv");
    renderable texturedSquare = makeSquare("texture.vert.spv", "texture.frag.spv");
    renderable renderables[] = { texturedSquare, coloredSquare };

	modelViewProj mvp;
	vec3 axis = {0, 0, 1};
	vec3 eye = {1, 1, 2};
	vec3 center = { 0, 0, 0};
	vec3 up = { 0, 0, 1 };
	glm_lookat(eye, center, up, mvp.view);
	glm_perspective(glm_rad(45), width / height, 0.01f, 10, mvp.projection);
	mvp.projection[1][1] *= -1;

	while (denymKeepRunning())
	{
        float elapsed_since_start = getUptime();

		vec3 down = { 0, 0, -0.5f };
	    glm_mat4_identity(mvp.model);
		glm_translate(mvp.model, down);
		glm_rotate(mvp.model, -glm_rad(elapsed_since_start * 100), axis);
        updateUniformsBuffer(coloredSquare, &mvp);

		glm_mat4_identity(mvp.model);
		glm_rotate(mvp.model, glm_rad(elapsed_since_start * 50), axis);
		updateUniformsBuffer(texturedSquare, &mvp);

		denymRender(renderables, 2);
		denymWaitForNextFrame();
	}

    denymDestroyRenderable(texturedSquare);
	denymDestroyRenderable(coloredSquare);
	denymTerminate();

	return EXIT_SUCCESS;
}
