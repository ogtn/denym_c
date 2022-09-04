#include "denym.h"
#include <stdlib.h>
#include <time.h>


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

	geometry geometry = denymCreateGeometry(6);
	denymGeometryAddPosition(geometry, positions);
	denymGeometryAddColors(geometry, colors);
    denymGeometryAddIndices(geometry, indices);
	
	renderable square = denymCreateRenderable(
		geometry,
		"mvp_ubo_position_color_attribute.vert.spv",
		"basic_color_interp.frag.spv");

	modelViewProj mvp;
	vec3 axis = {0, 0, 1};
	glm_mat4_identity(mvp.model);
	vec3 eye = {2, 2, 2};
	vec3 center = { 0, 0, 0};
	vec3 up = { 0, 0, 1 };
	glm_lookat(eye, center, up, mvp.view);
	glm_perspective(glm_rad(45), width / height, 0.01f, 10, mvp.projection);
	mvp.projection[1][1] *= -1;
	
	struct timespec start, now;
	timespec_get(&start, TIME_UTC);

	while (denymKeepRunning())
	{
		timespec_get(&now, TIME_UTC);
		uint64_t duration = now.tv_nsec / 1000000 + now.tv_sec - start.tv_nsec / 1000000 + start.tv_sec;
		glm_rotate(mvp.model, glm_rad(duration * 0.0000000001), axis);
		updateUniformsBuffer(square, &mvp);

		denymRender(square);
		denymWaitForNextFrame();
	}
	
	denymDestroyRenderable(square);
	denymTerminate();

	return EXIT_SUCCESS;
}
