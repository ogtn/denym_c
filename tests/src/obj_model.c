#include "camera_update.h"

#include <stdlib.h>


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
		.sendMVP = 1
	};
	renderable model = modelLoad("viking_room.obj", &params, 1, 0, 1, 0);
	renderableSetPosition(model, 0, -1, 0);

	params.textureName = "missing.png";
	renderable model2 = modelLoad("sphere.obj", &params, 1, 1, 1, 0);
	renderableSetPosition(model2, 0, 1, 0);

	vec3 eye = {4, 0, 2};
	vec3 center = { 0, 0, 0.5};

	input_t input;
	camera camera = cameraCreatePerspective(60, 0.01f, 1000.f);
	cameraLookAt(camera, eye, center);
	sceneSetCamera(denymGetScene(), camera);
	primitiveCreateGrid(8, 3);

	while(denymKeepRunning(&input))
	{
		float angularSpeed = denymGetTimeSinceLastFrame() * 20;

		renderableRotateZ(model, angularSpeed);
		renderableRotateZ(model2, angularSpeed);

		updateCameraPerspective(&input, camera);
		denymRender();
		denymWaitForNextFrame();
	}

	denymTerminate();

	return EXIT_SUCCESS;
}
