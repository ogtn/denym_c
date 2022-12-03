#include "camera_update.h"

#include <stdlib.h>


static void setModelMatrix(renderable renderable, float x, float y, float angle)
{
	mat4 matrix;

	glm_mat4_identity(matrix);
	glm_translate_x(matrix, x);
	glm_translate_y(matrix, y);
	glm_rotate_z(matrix, angle, matrix);
	renderableSetMatrix(renderable, matrix);
}


int main(void)
{
	const int width = 640;
	const int height = 480;

	if (denymInit(width, height))
		return EXIT_FAILURE;

	vec3 eye = {0, -5, 5};
	vec3 center = { 0, 0, 0};
	input_t input;
	camera camera = cameraCreatePerspective(60, 0.01f, 1000.f);
	cameraLookAt(camera, eye, center);
	sceneSetCamera(denymGetScene(), camera);

	renderableCreateParams params = {
		.textureName = "white.png",
		.vertShaderName = "gouraud.vert.spv",
		.fragShaderName = "gouraud.frag.spv",
		.sendMVP = 1,
        .sendLigths = 1
	};

    mat4 matrix;
    renderable floor_gouraud = primitiveCreateCube(10, 5, &params, 1);
    glm_mat4_identity(matrix);
	glm_translate_z(matrix, -0.5f);
    vec3 v = {0.5, 1, 1.f / 10};
    glm_scale(matrix, v);
    glm_translate_x(matrix, 5);
    renderableSetMatrix(floor_gouraud, matrix);

    renderable sphere_gouraud = primitiveCreateSphere(1.5f, 5, &params, 1);
    glm_mat4_identity(matrix);
	glm_translate_x(matrix, 1);
    glm_translate_z(matrix, 0.75f);
    renderableSetMatrix(sphere_gouraud, matrix);

    params.vertShaderName = "blinn_phong.vert.spv";
	params.fragShaderName = "blinn_phong.frag.spv";

    renderable floor_blinn = primitiveCreateCube(10, 5, &params, 1);
    glm_mat4_identity(matrix);
	glm_translate_z(matrix, -0.5f);
    glm_scale(matrix, v);
    glm_translate_x(matrix, -5);
    renderableSetMatrix(floor_blinn, matrix);

    renderable sphere_blinn = primitiveCreateSphere(1.5f, 5, &params, 1);
    glm_mat4_identity(matrix);
	glm_translate_x(matrix, -1);
    glm_translate_z(matrix, 0.75);
    renderableSetMatrix(sphere_blinn, matrix);

    params.vertShaderName = "texture_v4.vert.spv";
	params.fragShaderName = "texture_v3.frag.spv";
    params.sendMVPAsPushConstant = 1;
    params.compactMVP = 1;
    params.sendLigths = 0;
    renderable light = primitiveCreateSphere(0.2f, 3, &params, 1);

	while (denymKeepRunning(&input))
	{
		float angle = glm_rad(getUptime() * 200);

        glm_mat4_identity(matrix);
        vec3 position = {
            sin(angle) * 3,
            cos(angle) * 3,
            sin(glm_rad(getUptime() * 80)) * 2 + 2.5
        };
        glm_translate(matrix, position);
        renderableSetMatrix(light, matrix);
        sceneSetLightPosition(denymGetScene(), position);

		updateCameraPerspective(&input, camera);
		denymRender();
		denymWaitForNextFrame();
	}

	denymTerminate();

	return EXIT_SUCCESS;
}
