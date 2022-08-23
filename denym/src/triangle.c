#include "denym.h"
#include <stdlib.h>


int main(void)
{
	const int width = 640;
	const int height = 480;
	int result = EXIT_FAILURE;

	if (denymInit(width, height) == 0)
	{
		result = EXIT_SUCCESS;

		while (denymKeepRunning())
		{
			denymRender();
			denymWaitForNextFrame();
		}
	}

	denymTerminate();

	return result;
}
