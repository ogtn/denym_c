#include "camera_update.h"

#include <math.h>


int main(void)
{
	const int width = 640;
	const int height = 480;

	if (denymInit(width, height))
		return EXIT_FAILURE;

    sprite sprite = spriteCreate("mario_walk.png", 1, 2, 3, 2);

	vec3 eye = {0, -0.01f, 5};
	vec3 center = { 0, 0, 0};
    camera camera = cameraCreateOrtho(80, 0.1f, 1000.f);
	cameraLookAt(camera, eye, center);
	sceneSetCamera(denymGetScene(), camera);
    primitiveCreateGrid(8, 3);

	uint32_t spriteConfig[] = { 2, 3 };

	while(denymKeepRunning(NULL))
	{
		float time = getUptime();
		float pos = sinf(time * 1.5f) * 4 - 0.5f;
		int currentSprite = (int)(time / 5) % 2;

		spriteFlip(sprite, pos < 0, 0);
		spriteSetPosition(sprite, pos, 0);
		spriteSetSpriteCoordinates(sprite, (uint32_t)(time * 10) % spriteConfig[currentSprite], currentSprite);
		denymRender();
		denymWaitForNextFrame();
	}

	denymTerminate();

	return EXIT_SUCCESS;
}
