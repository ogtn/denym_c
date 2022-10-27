#include "denym.h"
#include "grid.h"

#include <stdlib.h>


#define ARRAY_SIZE 8


int main(void)
{
	const int width = 640;
	const int height = 640;

	if (denymInit(width, height))
		return EXIT_FAILURE;

	renderable grid = createGrid(32, 5);
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

	for(uint32_t i = 0; i < ARRAY_SIZE; i++)
    	for(uint32_t j = 0; j < ARRAY_SIZE; j++)
			for(uint32_t k = 0; k < ARRAY_SIZE; k++)
	        	objects[i][j][k] = modelLoad("sphere.obj", &params, 0, 0);

	printf("%d objects loaded in %.3fs\n",
		ARRAY_SIZE * ARRAY_SIZE * ARRAY_SIZE,
		getUptime() - start);

	vec3 eye = {16, 16, 16};
	vec3 center = { 0, 0, 0};
	camera camera = cameraCreatePerspective(60, 0.01f, 1000.f);
	cameraLookAt(camera, eye, center);
	sceneSetCamera(denymGetScene(), camera);

	while (denymKeepRunning())
	{
		float elapsed_since_start = getUptime();
		mat4 matrix;
		glm_mat4_identity(matrix);
		renderableSetMatrix(grid, matrix);

		for(uint32_t i = 0; i < ARRAY_SIZE; i++)
        {
        	for(uint32_t j = 0; j < ARRAY_SIZE; j++)
            {
				for(uint32_t k = 0; k < ARRAY_SIZE; k++)
				{
					float start = -(float)(ARRAY_SIZE) + 1;

					glm_mat4_identity(matrix);
					glm_translate_x(matrix, start + i * 2);
					glm_translate_y(matrix, start + j * 2);
					glm_translate_z(matrix, start + k * 2);
					glm_rotate_z(matrix, glm_rad(elapsed_since_start * 20), matrix);
					renderableSetMatrix(objects[i][j][k], matrix);
				}
            }
        }

		denymRender();
		denymWaitForNextFrame();
	}

	denymTerminate();

	return EXIT_SUCCESS;
}
