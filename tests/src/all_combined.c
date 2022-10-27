#include "denym.h"
#include "grid.h"

#include <stdlib.h>


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

	renderable grid = createGrid(8, 3);
	mat4 matrix;
	glm_mat4_identity(matrix);
	renderableSetMatrix(grid, matrix);

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
	params.textureName = "holes.png";
	renderable sphere_holes = modelLoad("sphere.obj", &params, 1, 0);
	params.textureName = "this file doesn't exist";
	renderable sphere_missing_texture = modelLoad("sphere.obj", &params, 1, 0);

	while (denymKeepRunning())
	{
		float angle = glm_rad(getUptime() * 20);

		glm_mat4_identity(matrix);
		glm_translate_x(matrix, -3);
		glm_translate_y(matrix, -3);
		glm_rotate_z(matrix, angle, matrix);
		renderableSetMatrix(room, matrix);

		glm_mat4_identity(matrix);
		glm_translate_x(matrix, -1);
		glm_translate_y(matrix, -3);
		glm_rotate_z(matrix, angle, matrix);
		renderableSetMatrix(room_indexed, matrix);

		glm_mat4_identity(matrix);
		glm_translate_x(matrix, 1);
		glm_translate_y(matrix, -3);
		glm_rotate_z(matrix, angle, matrix);
		renderableSetMatrix(room_normals, matrix);

		glm_mat4_identity(matrix);
		glm_translate_x(matrix, 3);
		glm_translate_y(matrix, -3);
		glm_rotate_z(matrix, angle, matrix);
		renderableSetMatrix(room_indexed_normals, matrix);

		glm_mat4_identity(matrix);
		glm_translate_x(matrix, -3);
		glm_translate_y(matrix, -1);
		glm_rotate_z(matrix, angle, matrix);
		renderableSetMatrix(room_compact_mvp, matrix);

		glm_mat4_identity(matrix);
		glm_translate_x(matrix, -1);
		glm_translate_y(matrix, -1);
		glm_rotate_z(matrix, angle, matrix);
		renderableSetMatrix(room_push_cst_mvp, matrix);

		glm_mat4_identity(matrix);
		glm_translate_x(matrix, 1);
		glm_translate_y(matrix, -1);
		glm_rotate_z(matrix, angle, matrix);
		renderableSetMatrix(sphere_holes, matrix);

		glm_mat4_identity(matrix);
		glm_translate_x(matrix, 3);
		glm_translate_y(matrix, -1);
		glm_rotate_z(matrix, angle, matrix);
		renderableSetMatrix(sphere_missing_texture, matrix);

		denymRender();
		denymWaitForNextFrame();
	}

	denymTerminate();

	return EXIT_SUCCESS;
}
