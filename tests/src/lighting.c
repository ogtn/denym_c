#include "camera_update.h"

#include <stdlib.h>

typedef struct orb
{
	renderable renderable;
	plight light;
} orb;


static void initOrb(orb *orb, scene scene, color color)
{
	material_t mtl = {	.color = color };
	renderableCreateParams params = {
		.vertShaderName = "mtl_no_light.vert.spv",
		.fragShaderName = "mtl_no_light.frag.spv",
		.sendMVPAsPushConstant = 1,
		.material = &mtl
	};

	orb->renderable = primitiveCreateSphere(0.2f, 3, &params, 1, 0, 0);
	orb->light = sceneAddPointLight(scene);
	orb->light->color = color;
	orb->light->intensity = 0.5;
}


static void setOrbPosition(orb *orb, float x, float y, float z)
{
	mat4 matrix;
	vec3 position = { x, y, z };

	glm_mat4_identity(matrix);
	glm_translate(matrix, position);
	glm_vec3_copy(position, orb->light->position.v);
	renderableSetMatrix(orb->renderable, matrix);
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
	scene scene = denymGetScene();
	sceneSetCamera(scene, camera);

	renderableCreateParams params = {
		.vertShaderName = "gouraud_plain.vert.spv",
		.fragShaderName = "gouraud_plain.frag.spv",
		.sendMVP = 1,
		.sendLigths = 1
	};

	color white = { 1, 1, 1 };
	color green = { 0, 1, 0 };
	color blue = { 0, 0, 1 };

	material_t matte_white = {
		.color = white,
		.shininess = 8
	};

	material_t shiny_white = {
		.color = white,
		.shininess = 200
	};

	mat4 matrix;

	params.material = &shiny_white;
	renderable floor_gouraud = primitiveCreateCube(10, 5, &params, 1, 0, 1);
	glm_mat4_identity(matrix);
	glm_translate_z(matrix, -0.5f);
	vec3 v = {0.5, 1, 1.f / 10};
	glm_scale(matrix, v);
	glm_translate_x(matrix, 5);
	renderableSetMatrix(floor_gouraud, matrix);

	params.material = &matte_white;
	renderable sphere_gouraud = primitiveCreateSphere(1.5f, 5, &params, 1, 0, 1);
	glm_mat4_identity(matrix);
	glm_translate_x(matrix, 1);
	glm_translate_z(matrix, 0.75f);
	renderableSetMatrix(sphere_gouraud, matrix);

	params.vertShaderName = "blinn_phong_plain.vert.spv",
	params.fragShaderName = "blinn_phong_plain.frag.spv",

	params.material = &shiny_white;
	renderable floor_blinn = primitiveCreateCube(10, 5, &params, 1, 0, 1);
	glm_mat4_identity(matrix);
	glm_translate_z(matrix, -0.5f);
	glm_scale(matrix, v);
	glm_translate_x(matrix, -5);
	renderableSetMatrix(floor_blinn, matrix);

	params.material = &matte_white;
	renderable sphere_blinn = primitiveCreateSphere(1.5f, 5, &params, 1, 0, 1);
	glm_mat4_identity(matrix);
	glm_translate_x(matrix, -1);
	glm_translate_z(matrix, 0.75);
	renderableSetMatrix(sphere_blinn, matrix);

	orb orb1, orb2;
	initOrb(&orb1, scene, blue);
	initOrb(&orb2, scene, green);

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

			setOrbPosition(&orb1, sin(angle) * 3, cos(angle) * 3, sin(glm_rad(time * 80)) + 1.5);
			setOrbPosition(&orb2, cos(angle) * 3, sin(angle) * 3, cos(glm_rad(time * 80)) + 1.5);
		}

		updateCameraPerspective(&input, camera);
		denymRender();
		denymWaitForNextFrame();
	}

	denymTerminate();

	return EXIT_SUCCESS;
}
