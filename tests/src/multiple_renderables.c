#include "denym.h"
#include <stdlib.h>
#include <math.h>


static renderable makeTriangle(void)
{
    // clip coordinates
	float positions[] = 
	{
		0.0f, -0.5f,
		0.5f, 0.5f,
		-0.5f, 0.5f
	};

	float colors[] =
	{
		1, 0, 0,
		0, 1, 0,
		0, 0, 1
	};

	geometryCreateInfo geometryCreateInfo = {
		.vertexCount = 3,
		.positions = positions,
		.colors = colors };

	geometry geometry = geometryCreate(&geometryCreateInfo);
    renderable triangle = denymCreateRenderable(
        geometry,
        "basic_position_color_attribute.vert.spv",
        "basic_color_interp.frag.spv");

    return triangle;
}


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

	geometryCreateInfo geometryCreateInfo = {
		.vertexCount = 6,
		.positions = positions,
		.colors = colors,
		.indices = indices,
		.indiceCount = sizeof indices / sizeof *indices };

	geometry geometry = geometryCreate(&geometryCreateInfo);
	renderable square = denymCreateRenderable(
		geometry,
		"mvp_ubo_position_color_attribute.vert.spv",
		"basic_color_interp.frag.spv");

    useUniforms(square);

    return square;
}


int main(void)
{
	const int width = 640;
	const int height = 640;

	if (denymInit(width, height))
		return EXIT_FAILURE;

    renderable square1 = makeSquare();
    renderable square = makeSquare();
    renderable renderables[] = { square1, square };

	modelViewProj mvp;
	vec3 axis = {0, 0, 1};
	vec3 eye = {2, 2, 2};
	vec3 center = { 0, 0, 0};
	vec3 up = { 0, 0, 1 };
	glm_lookat(eye, center, up, mvp.view);
	glm_perspective(glm_rad(45), width / height, 0.01f, 10, mvp.projection);
	mvp.projection[1][1] *= -1;

	while (denymKeepRunning())
	{
        float elapsed_since_start = getUptime();

	    glm_mat4_identity(mvp.model);
        updateUniformsBuffer(square1, &mvp);

		glm_rotate(mvp.model, glm_rad(elapsed_since_start * 100), axis);
		updateUniformsBuffer(square, &mvp);

		denymRender(renderables, 2);
		denymWaitForNextFrame();
	}

    denymDestroyRenderable(square1);
	denymDestroyRenderable(square);
	denymTerminate();

	return EXIT_SUCCESS;
}
