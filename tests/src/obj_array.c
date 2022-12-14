#include "camera_update.h"

#include <stdlib.h>


#define ARRAY_SIZE 8


int main(void)
{
	const int width = 640;
	const int height = 640;

	if (denymInit(width, height))
		return EXIT_FAILURE;

	primitiveCreateGrid(32, 5);
    renderable objects[ARRAY_SIZE][ARRAY_SIZE][ARRAY_SIZE];

	renderableCreateParams params = {
		.textureName = "missing.png",
		.vertShaderName = "texture_v4.vert.spv",
		.fragShaderName = "texture_v3.frag.spv",
		.sendMVP = 1,
		.compactMVP = 1,
		.sendMVPAsPushConstant = 1
	};

	float start = getUptime();
	const float orig = -(float)(ARRAY_SIZE) + 1;

	for(uint32_t i = 0; i < ARRAY_SIZE; i++)
	{
		for(uint32_t j = 0; j < ARRAY_SIZE; j++)
		{
			for(uint32_t k = 0; k < ARRAY_SIZE; k++)
			{
				objects[i][j][k] = modelLoad("sphere.obj", &params, 1, 1, 1, 0);
				renderableSetPosition(objects[i][j][k], orig + i * 2, orig + j * 2, orig + k * 2);
			}
		}
	}

	printf("%d objects loaded in %.3fs\n",
		ARRAY_SIZE * ARRAY_SIZE * ARRAY_SIZE,
		getUptime() - start);

	vec3 eye = {16, 16, 16};
	vec3 center = { 0, 0, 0};
	input_t input;
	camera camera = cameraCreatePerspective(60, 0.01f, 1000.f);
	cameraLookAt(camera, eye, center);
	sceneSetCamera(denymGetScene(), camera);

	while(denymKeepRunning(&input))
	{
		float angularSpeed = denymGetTimeSinceLastFrame() * 20;

		for(uint32_t i = 0; i < ARRAY_SIZE; i++)
        	for(uint32_t j = 0; j < ARRAY_SIZE; j++)
				for(uint32_t k = 0; k < ARRAY_SIZE; k++)
					renderableRotateZ(objects[i][j][k], angularSpeed);

		updateCameraPerspective(&input, camera);
		denymRender();
		denymWaitForNextFrame();
	}

	denymTerminate();

	return EXIT_SUCCESS;
}
