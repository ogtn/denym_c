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

	vec3 eye = {2, 2, 2};
	vec3 center = { 0, 0, 0};

	input_t input;
	camera camera = cameraCreatePerspective(60, 0.01f, 1000.f);
	cameraLookAt(camera, eye, center);
	sceneSetCamera(denymGetScene(), camera);
	primitiveCreateGrid(8, 3);

    uint32_t dimension = 0;
    uint32_t transfo = 0;

	while(denymKeepRunning(&input))
	{
		float elapsed = denymGetTimeSinceLastFrame() * 5;
        float speed = 0;
        vec3f value = { 0, 0, 0 };
        vec3f axis = { 0, 0, 0 };

        // select the axis: x, y, z
        if(inputIsKeyPressed(INPUT_KEY_1))
            dimension = 0;
        if(inputIsKeyPressed(INPUT_KEY_2))
            dimension = 1;
        if(inputIsKeyPressed(INPUT_KEY_3))
            dimension = 2;

        // select the transformation: position, rotation, scale
        if(inputIsKeyPressed(INPUT_KEY_F1))
            transfo = 0;
        if(inputIsKeyPressed(INPUT_KEY_F2))
            transfo = 1;
        if(inputIsKeyPressed(INPUT_KEY_F3))
            transfo = 2;

        // increase, decrease or reset value
        if(inputIsKeyPressed(INPUT_KEY_PAGE_UP))
            speed = elapsed;
        if(inputIsKeyPressed(INPUT_KEY_DELETE))
            speed = 0;
        if(inputIsKeyPressed(INPUT_KEY_PAGE_DOWN))
            speed = -elapsed;

        value.v[dimension] = speed;
        axis.v[dimension] = 1;

        if(transfo == 0)
            renderableMoveV(model, &value);
        else if(transfo == 1)
            renderableRotateAxis(model, speed * 20, &axis);
        else if(transfo == 2)
            renderableScaleV(model, &value);

		updateCameraPerspective(&input, camera);
		denymRender();
		denymWaitForNextFrame();
	}

	denymTerminate();

	return EXIT_SUCCESS;
}
