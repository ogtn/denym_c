#include "denym.h"
#include <stdlib.h>
#include <math.h>


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
        0, 1, 2,
        2, 3, 0
    };

	if (denymInit(width, height))
		return EXIT_FAILURE;

	geometryCreateParams geometryCreateParams = {
		.vertexCount = 4,
		.positions2D = positions,
		.colors = colors,
		.indices_16 = indices,
		.indexCount = sizeof indices / sizeof *indices };

	renderableCreateParams renderableParams = {
		.geometry = geometryCreate(&geometryCreateParams),
		.vertShaderName = "mvp_ubo_position_color_attribute.vert.spv",
		.fragShaderName = "basic_color_with_alpha_cst.frag.spv",
		.useUniforms = 1,
		.usePushConstant = 1
	};

	renderable square = denymCreateRenderable(&renderableParams);

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
		glm_rotate(mvp.model, glm_rad(elapsed_since_start * 100), axis);
		updateUniformsBuffer(square, &mvp);
        float alpha = (sinf(elapsed_since_start * 4) + 1) / 2;
        updatePushConstants(square, alpha);

		denymRender(&square, 1);
		denymWaitForNextFrame();
	}

	denymDestroyRenderable(square);
	denymTerminate();

	return EXIT_SUCCESS;
}
