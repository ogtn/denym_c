#include "denym.h"

#include <stdlib.h>
#include <math.h>
#include <string.h>


static renderable createGrid(float size, uint32_t level)
{
    static const float black[] = {0, 0, 0};
    static const float grey[] = {0.2f, 0.2f, 0.2f};
    const float end = size / 2.f;
    const float start = -end;

    uint32_t subdivisions = 1 << level;
    uint32_t vertexCount = (subdivisions + 1) * 4;
    float *positions = malloc(sizeof(float) * 2 * vertexCount);
    float *colors = malloc(sizeof(float) * 3 * vertexCount);
    float step = size / (float)subdivisions;
    float *pos = positions;
    float *col = colors;

    for(uint32_t i = 0; i <= subdivisions; i++)
    {
        float current = start + (float)i * step;

        // x axis
        *pos++ = current;
        *pos++ = start;
        *pos++ = current;
        *pos++ = end;

        // y axis
        *pos++ = start;
        *pos++ = current;
        *pos++ = end;
        *pos++ = current;

        for(uint32_t j = 0; j < 4; j++)
        {
            memcpy(col, i == subdivisions / 2 ? black : grey, sizeof black);
            col += 3;
        }
    }

    geometryParams geometryParams = geometryCreateParameters(vertexCount, 0);
    geometryParamsAddAttribVec2(geometryParams, positions);
    geometryParamsAddAttribVec3(geometryParams, colors);

    renderableCreateParams renderableParams = {
        .vertShaderName = "grid.vert.spv",
        .fragShaderName = "grid.frag.spv",
        .useWireFrame = 1,
        .geometry = geometryCreate(geometryParams),
        .uniformSize = sizeof(modelViewProj)
    };

    free(positions);
    free(colors);

    return denymCreateRenderable(&renderableParams);
}


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
		.uniformSize = sizeof(modelViewProj)
	};

	renderable model = modelLoad("viking_room.obj", &params, 0, 0);
	params.textureName = "missing.png";
	renderable model2 = modelLoad("sphere.obj", &params, 1, 1);
	renderable grid = createGrid(8, 3);

	vec3 eye = {4, 0, 2};
	vec3 center = { 0, 0, 0.5};

	camera camera = cameraCreatePerspective(45, (float)width / (float)height, 0.01f, 1000.f);
	cameraLookAt(camera, eye, center);
	sceneSetCamera(denymGetScene(), camera);

	mat4 matrix;
	glm_mat4_identity(matrix);
	renderableSetMatrix(grid, matrix);

	while (denymKeepRunning())
	{
		float elapsed_since_start = getUptime();

		glm_mat4_identity(matrix);
		glm_translate_y(matrix, -1);
		glm_rotate_z(matrix, glm_rad(elapsed_since_start * 20), matrix);
		renderableSetMatrix(model, matrix);

		glm_mat4_identity(matrix);
		glm_translate_y(matrix, 1);
		glm_rotate_z(matrix, glm_rad(elapsed_since_start * 20), matrix);
		renderableSetMatrix(model2, matrix);

		denymRender();
		denymWaitForNextFrame();
	}

	denymTerminate();

	return EXIT_SUCCESS;
}
