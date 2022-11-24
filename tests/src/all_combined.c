#include "denym.h"

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

	vec3 eye = {0, 5, 5};
	vec3 center = { 0, 0, 0.5};
	camera camera = cameraCreatePerspective(60, 0.01f, 1000.f);
	cameraLookAt(camera, eye, center);
	sceneSetCamera(denymGetScene(), camera);

	renderableCreateParams params = {
		.textureName = "viking_room.png",
		.vertShaderName = "texture_v2.vert.spv",
		.fragShaderName = "texture_v2.frag.spv",
		.sendMVP = 1
	};
	renderable room = modelLoad("viking_room.obj", &params, 0, 0);
	renderable room_indexed = modelLoad("viking_room.obj", &params, 1, 0);
	renderable room_normals = modelLoad("viking_room.obj", &params, 0, 1);
	renderable room_indexed_normals = modelLoad("viking_room.obj", &params, 1, 1);
	params.compactMVP = 1;
	params.vertShaderName = "texture_v3.vert.spv";
	renderable room_compact_mvp = modelLoad("viking_room.obj", &params, 1, 0);
	params.sendMVPAsPushConstant = 1;
	params.vertShaderName = "texture_v4.vert.spv";
	params.fragShaderName = "texture_v3.frag.spv";
	renderable room_push_cst_mvp = modelLoad("viking_room.obj", &params, 1, 0);
	params.vertShaderName = "texture_v5.vert.spv";
	params.fragShaderName = "texture_v2.frag.spv";
	params.sendMVPAsPushConstant = 0;
	params.sendMVPAsStorageBuffer = 1;
	renderable room_sbbo = modelLoad("viking_room.obj", &params, 1, 0);
	params.vertShaderName = "texture_v4.vert.spv";
	params.fragShaderName = "texture_v3.frag.spv";
	params.sendMVPAsPushConstant = 1;
	params.sendMVPAsStorageBuffer = 0;
	params.textureName = "holes.png";
	renderable sphere_holes = modelLoad("sphere.obj", &params, 1, 0);
	params.textureName = "this file doesn't exist";
	renderable sphere_missing_texture = modelLoad("sphere.obj", &params, 1, 0);
	params.sendMVPAsPushConstant = 0;
	params.compactMVP = 0;
	params.textureName = "debug.png",
	params.vertShaderName = "gouraud.vert.spv";
	params.fragShaderName = "gouraud.frag.spv";
	renderable cube_gouraud = primitiveCreateCube(1.5f, 4, &params);
	renderable sphere_gouraud = primitiveCreateSphere(2, 4, &params);
	params.vertShaderName = "blinn_phong.vert.spv";
	params.fragShaderName = "blinn_phong.frag.spv";
	renderable sphere_blinn = primitiveCreateSphere(2, 4, &params);

	primitiveCreateGrid(8, 3);

	while (denymKeepRunning())
	{
		float angle = glm_rad(getUptime() * 20);

		setModelMatrix(room, -3, -3, angle);
		setModelMatrix(room_indexed, -1, -3, angle);
		setModelMatrix(room_normals, 1, -3, angle);
		setModelMatrix(room_indexed_normals, 3, -3, angle);
		setModelMatrix(room_compact_mvp, -3, -1, angle);
		setModelMatrix(room_push_cst_mvp, -1, -1, angle);
		setModelMatrix(room_sbbo, 1, -1, angle);
		setModelMatrix(sphere_holes, 3, -1, angle);
		setModelMatrix(sphere_missing_texture, -3, 1, angle);
		setModelMatrix(cube_gouraud, -1, 1, angle);
		setModelMatrix(sphere_gouraud, 1, 1, angle);
		setModelMatrix(sphere_blinn, 3, 1, angle);

		denymRender();
		denymWaitForNextFrame();
	}

	denymTerminate();

	return EXIT_SUCCESS;
}
