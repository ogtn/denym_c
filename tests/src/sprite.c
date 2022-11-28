#include "denym.h"

#include <stdlib.h>


int main(void)
{
	const int width = 640;
	const int height = 480;

	if (denymInit(width, height))
		return EXIT_FAILURE;

    sprite sprite = spriteCreate("mario_walk.png", 1, 2, 3, 1);

	vec3 eye = {0, -0.01, 5};
	vec3 center = { 0, 0, 0};
    camera camera = cameraCreateOrtho(60, 0.1f, 1000.f);
    camera = cameraCreatePerspective(60, 0.1f, 1000);
	cameraLookAt(camera, eye, center);
	sceneSetCamera(denymGetScene(), camera);
    primitiveCreateGrid(8, 3);

	while (denymKeepRunning())
	{
		spriteSetSpriteCoordinates(sprite, (int)(getUptime() * 10) % 3, 0);
		denymRender();
		denymWaitForNextFrame();
	}

	denymTerminate();

	return EXIT_SUCCESS;
}
