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
		.uniformSize = sizeof(uniforms)
	};

	renderable model = modelLoad("viking_room.obj", &params, 0, 0);
	params.textureName = "missing.png";
	renderable model2 = modelLoad("sphere.obj", &params, 1, 1);
	renderable grid = createGrid(8, 3);

	renderable models[] = {grid, model, model2};
	uniforms uniforms;
	vec3 axis = {0, 0, 1};
	vec3 eye = {4, 0, 2};
	vec3 center = { 0, 0, 0.5};
	vec3 up = { 0, 0, 1 };
	glm_lookat(eye, center, up, uniforms.view);
	glm_perspective(glm_rad(45), (float)width / height, 0.01f, 1000, uniforms.projection);
	uniforms.projection[1][1] *= -1;
	glm_mat4_identity(uniforms.model);
	updateUniformsBuffer(grid, &uniforms);

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

		denymRender(models, sizeof models / sizeof *models);
		denymWaitForNextFrame();
	}

	for(uint32_t i =  0; i < sizeof models / sizeof *models; i++)
		denymDestroyRenderable(models[i]);

	denymTerminate();

	return EXIT_SUCCESS;
}
