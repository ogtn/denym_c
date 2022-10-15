#include "denym.h"

#include <stdlib.h>
#include <math.h>
#include <string.h>


typedef struct uniforms
{
	mat4 model;
	mat4 view;
	mat4 projection;
} uniforms;


int main(void)
{
	const int width = 640;
	const int height = 480;

	if (denymInit(width, height))
		return EXIT_FAILURE;

	renderableCreateParams params = {
		.textureName = "viking_room.png",
		.vertShaderName = "texture_v2.vert.spv",
		.fragShaderName = "texture_v2.frag.spv",
		.uniformSize = sizeof(uniforms)
	};

	renderable model = modelLoad("viking_room.obj", &params, 0, 0);

	params.textureName = "missing.png";
	renderable model2 = modelLoad("sphere.obj", &params, 1, 1);

	renderable models[] = {model, model2};
	uniforms uniforms;
	vec3 axis = {0, 0, 1};
	vec3 eye = {4, 0, 2};
	vec3 center = { 0, 0, 0.5};
	vec3 up = { 0, 0, 1 };
	glm_lookat(eye, center, up, uniforms.view);
	glm_perspective(glm_rad(45), (float)width / height, 0.01f, 1000, uniforms.projection);
	uniforms.projection[1][1] *= -1;

	while (denymKeepRunning())
	{
		float elapsed_since_start = getUptime();

		glm_mat4_identity(uniforms.model);
		glm_translate_y(uniforms.model, -1);
		glm_rotate(uniforms.model, glm_rad(elapsed_since_start * 20), axis);
		updateUniformsBuffer(model, &uniforms);

		glm_mat4_identity(uniforms.model);
		glm_translate_y(uniforms.model, 1);
		glm_rotate(uniforms.model, glm_rad(elapsed_since_start * 20), axis);
		updateUniformsBuffer(model2, &uniforms);

		denymRender(models, 2);
		denymWaitForNextFrame();
	}

	denymDestroyRenderable(model);
	denymDestroyRenderable(model2);
	denymTerminate();

	return EXIT_SUCCESS;
}
