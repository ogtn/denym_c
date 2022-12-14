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
	sceneSetCamera(denymGetScene(), camera);
	renderable models[16] = { NULL };
	uint32_t modelsCount = 0;

	renderableCreateParams params = {
		.textureName = "viking_room.png",
		.vertShaderName = "texture_v2.vert.spv",
		.fragShaderName = "texture_v2.frag.spv",
		.sendMVP = 1
	};

	renderable room = models[modelsCount++] = modelLoad("viking_room.obj", &params, 1, 0, 1, 0);
	renderable room_indexed = models[modelsCount++] = modelLoad("viking_room.obj", &params, 1, 1, 1, 0);
	renderable room_normals = models[modelsCount++] = modelLoad("viking_room.obj", &params, 1, 0, 1, 1);
	renderable room_indexed_normals = models[modelsCount++] = modelLoad("viking_room.obj", &params, 1, 1, 1, 1);
	params.compactMVP = 1;
	params.vertShaderName = "texture_v3.vert.spv";
	renderable room_compact_mvp = models[modelsCount++] = modelLoad("viking_room.obj", &params, 1, 1, 1, 0);
	params.sendMVPAsPushConstant = 1;
	params.vertShaderName = "texture_v4.vert.spv";
	params.fragShaderName = "texture_v3.frag.spv";
	renderable room_push_cst_mvp = models[modelsCount++] = modelLoad("viking_room.obj", &params, 1, 1, 1, 0);
	params.vertShaderName = "texture_v5.vert.spv";
	params.fragShaderName = "texture_v2.frag.spv";
	params.sendMVPAsPushConstant = 0;
	params.sendMVPAsStorageBuffer = 1;
	renderable room_sbbo = models[modelsCount++] = modelLoad("viking_room.obj", &params, 1, 1, 1, 0);
	params.vertShaderName = "texture_v4.vert.spv";
	params.fragShaderName = "texture_v3.frag.spv";
	params.sendMVPAsPushConstant = 1;
	params.sendMVPAsStorageBuffer = 0;
	params.textureName = "holes.png";
	renderable sphere_holes = models[modelsCount++] = modelLoad("sphere.obj", &params, 1, 1, 1, 0);
	params.textureName = "this file doesn't exist";
	renderable sphere_missing_texture = models[modelsCount++] = modelLoad("sphere.obj", &params, 1, 1, 1, 0);
	params.sendMVPAsPushConstant = 0;
	params.compactMVP = 0;
	params.textureName = "debug.png",
	params.vertShaderName = "gouraud.vert.spv";
	params.fragShaderName = "gouraud.frag.spv";
	params.sendLigths = 1;
	renderable cube_gouraud = models[modelsCount++] = primitiveCreateCube(1.5f, 4, &params, 1, 1, 1);
	renderable sphere_gouraud = models[modelsCount++] = primitiveCreateSphere(2, 4, &params, 1, 1, 1);
	params.vertShaderName = "blinn_phong.vert.spv";
	params.fragShaderName = "blinn_phong.frag.spv";
	renderable sphere_blinn = models[modelsCount++] = primitiveCreateSphere(2, 4, &params, 1, 1, 1);

	for(int32_t id = 0; id < modelsCount; id++)
		renderableSetPosition(models[id], (id % 4) * 2 - 3, 3 - (id / 4 * 2), 0);

	dlight light = sceneAddDirectionalLight(denymGetScene());
	light->intensity = 1;
	primitiveCreateGrid(8, 3);

	while (denymKeepRunning(&input))
	{
		float angularSpeed = denymGetTimeSinceLastFrame() * 20;

		for(int32_t id = 0; id < modelsCount; id++)
			renderableRotateZ(models[id], angularSpeed);

		updateCameraPerspective(&input, camera);
		denymRender();
		denymWaitForNextFrame();
	}

	denymTerminate();

	return EXIT_SUCCESS;
}
