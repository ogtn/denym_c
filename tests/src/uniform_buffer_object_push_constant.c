#include "denym.h"
#include <stdlib.h>
#include <time.h>
#include <math.h>


static void timespec_diff(const struct timespec *lhs, const struct timespec *rhs, struct timespec *result)
{
    result->tv_sec = lhs->tv_sec - rhs->tv_sec;
    result->tv_nsec = lhs->tv_nsec - rhs->tv_nsec;

    if (result->tv_nsec < 0)
    {
        result->tv_sec--;
        result->tv_nsec += 1000000000L;
    }
}


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
		"basic_color_with_alpha_cst.frag.spv");

    usePushConstants(square);
    useUniforms(square);

	modelViewProj mvp;
	vec3 axis = {0, 0, 1};
	vec3 eye = {2, 2, 2};
	vec3 center = { 0, 0, 0};
	vec3 up = { 0, 0, 1 };
	glm_lookat(eye, center, up, mvp.view);
	glm_perspective(glm_rad(45), width / height, 0.01f, 10, mvp.projection);
	mvp.projection[1][1] *= -1;

	struct timespec start, now, diff;
	timespec_get(&start, TIME_UTC);

	while (denymKeepRunning())
	{
		timespec_get(&now, TIME_UTC);
        timespec_diff(&now, &start, &diff);
        float elapsed_since_start = (float)diff.tv_sec + (float)diff.tv_nsec / 1000000000.f;

	    glm_mat4_identity(mvp.model);
		glm_rotate(mvp.model, glm_rad(elapsed_since_start * 100), axis);
		updateUniformsBuffer(square, &mvp);
        float alpha = (sinf(elapsed_since_start * 4) + 1) / 2;
        updatePushConstants(square, alpha);

		denymRender(square);
		denymWaitForNextFrame();
	}
	
	denymDestroyRenderable(square);
	denymTerminate();

	return EXIT_SUCCESS;
}
