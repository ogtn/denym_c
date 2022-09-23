#include "denym.h"

#include <stdlib.h>
#include <math.h>
#include <string.h>


int main(void)
{
	const int width = 640;
	const int height = 480;

	if (denymInit(width, height))
		return EXIT_FAILURE;

	renderable model = modelLoad(
		"viking_room.obj",
		"viking_room.png",
		"texture_v2.vert.spv",
		"texture_v2.frag.spv");

	renderable model2 = modelLoad(
		"viking_room.obj",
		"viking_room.png",
		"texture_v2.vert.spv",
		"texture_v2.frag.spv");

	renderable models[] = {model, model2};
	modelViewProj mvp;
	vec3 axis = {0, 0, 1};
	vec3 eye = {4, 0, 2};
	vec3 center = { 0, 0, 0.5};
	vec3 up = { 0, 0, 1 };
	glm_lookat(eye, center, up, mvp.view);
	glm_perspective(glm_rad(45), (float)width / height, 0.01f, 1000, mvp.projection);
	mvp.projection[1][1] *= -1;

	while (denymKeepRunning())
	{
		float elapsed_since_start = getUptime();

		glm_mat4_identity(mvp.model);
		glm_translate_y(mvp.model, -1);
		glm_rotate(mvp.model, glm_rad(elapsed_since_start * 20), axis);
		updateUniformsBuffer(model, &mvp);

		glm_mat4_identity(mvp.model);
		glm_translate_y(mvp.model, 1);
		glm_rotate(mvp.model, glm_rad(elapsed_since_start * 20), axis);
		updateUniformsBuffer(model2, &mvp);

		denymRender(models, 2);
		denymWaitForNextFrame();
	}

	denymDestroyRenderable(model);
	denymDestroyRenderable(model2);
	denymTerminate();

	return EXIT_SUCCESS;
}
