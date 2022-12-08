#include "camera_update.h"

#include <stdlib.h>


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
	scene scene = denymGetScene();
	sceneSetCamera(scene, camera);

	renderableCreateParams params = {
		.textureName = "white.png",
		.vertShaderName = "gouraud.vert.spv",
		.fragShaderName = "gouraud.frag.spv",
		.sendMVP = 1,
        .sendLigths = 1
	};

	material_t matte_white = {
		.color.r = 1,
		.color.g = 1,
		.color.b = 1,
		.shininess = 1
	};

	material_t shiny_white = {
		.color.r = 1,
		.color.g = 1,
		.color.b = 1,
		.shininess = 200
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

	params.material = &matte_white;
    renderable floor_blinn = primitiveCreateCube(10, 5, &params, 1);
    glm_mat4_identity(matrix);
	glm_translate_z(matrix, -0.5f);
    glm_scale(matrix, v);
    glm_translate_x(matrix, -5);
    renderableSetMatrix(floor_blinn, matrix);

	params.material = &shiny_white;
    renderable sphere_blinn = primitiveCreateSphere(1.5f, 5, &params, 1);
    glm_mat4_identity(matrix);
	glm_translate_x(matrix, -1);
    glm_translate_z(matrix, 0.75);
    renderableSetMatrix(sphere_blinn, matrix);
	params.material = NULL;

    params.vertShaderName = "texture_v4.vert.spv";
	params.fragShaderName = "texture_v3.frag.spv";
    params.sendMVPAsPushConstant = 1;
    params.compactMVP = 1;
    params.sendLigths = 0;
    renderable light = primitiveCreateSphere(0.2f, 3, &params, 1);
	renderable light2 = primitiveCreateSphere(0.2f, 3, &params, 1);

	dlight dlight = sceneAddDirectionalLight(scene);
    plight plight1 = sceneAddPointLight(scene);
	plight1->color.r = plight1->color.g = 0;
	plight plight2 = sceneAddPointLight(scene);
	plight2->color.r = plight2->color.b = 0;

	int pause = 0;
	int previousP = 0;
	float pauseTime;
	float delay = 0;

	while (denymKeepRunning(&input))
	{
		if(inputIsKeyPressed(INPUT_KEY_P))
		{
			if(!previousP)
			{
				if(pause)
				{
					delay += getUptime() - pauseTime;
					pause = 0;
				}
				else
				{
					pauseTime = getUptime();
					pause = 1;
				}
			}

			previousP = 1;
		}
		else
			previousP = 0;

		if(!pause)
		{
			float time = getUptime() - delay;
			float angle = glm_rad(time * 200);

			glm_mat4_identity(matrix);
			vec3 position = {
				sin(angle) * 3,
				cos(angle) * 3,
				sin(glm_rad(time * 80)) + 1.5
			};
			glm_translate(matrix, position);
			glm_vec3_copy(position, plight1->position.v);
			renderableSetMatrix(light, matrix);

			glm_mat4_identity(matrix);
			position[0] = cos(angle) * 3;
			position[1] = sin(angle) * 3;
			position[2] = cos(glm_rad(time * 80)) + 1.5;
			glm_translate(matrix, position);
			glm_vec3_copy(position, plight2->position.v);
			renderableSetMatrix(light2, matrix);
		}

		updateCameraPerspective(&input, camera);
		denymRender();
		denymWaitForNextFrame();
	}

	denymTerminate();

	return EXIT_SUCCESS;
}
