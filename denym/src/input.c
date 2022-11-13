#include "input.h"
#include "core.h"
#include "logger.h"

#include <string.h>


static void inputGlfwKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);

static void inputGlfwScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

static void inputUpdateMouse(void);

int inputCreate(void)
{
    memset(&engine.input, 0, sizeof engine.input);
    glfwSetKeyCallback(engine.window, inputGlfwKeyCallback);
    glfwSetScrollCallback(engine.window, inputGlfwScrollCallback);

    if(engine.settings.captureMouse)
    {
        glfwSetInputMode(engine.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        if(glfwRawMouseMotionSupported())
            glfwSetInputMode(engine.window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }

    return 0;
}


void inputUpdate(void)
{
    inputUpdateMouse();
    glfwPollEvents();
}


static void inputGlfwKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
	if(key == GLFW_KEY_F && action == GLFW_PRESS)
	{
		if(engine.isFullScreen)
		{
			glfwSetWindowMonitor(engine.window, NULL, engine.windowPosX, engine.windowPosY, engine.windowWidth, engine.windowHeight, GLFW_DONT_CARE);
			engine.isFullScreen = VK_FALSE;
		}
		else
		{
			int width, height;
			GLFWmonitor *monitor = glfwGetPrimaryMonitor();
			glfwGetMonitorWorkarea(monitor, NULL, NULL, &width, &height);
			glfwGetWindowPos(engine.window, &engine.windowPosX, &engine.windowPosY);
			glfwGetWindowSize(engine.window, &engine.windowWidth, &engine.windowHeight);
			glfwSetWindowMonitor(engine.window, monitor, 0, 0, width, height, GLFW_DONT_CARE);
			engine.isFullScreen = VK_TRUE;
		}
	}
}


static void inputGlfwScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    float fov =
    engine.input.mouse.scroll.x += xoffset;
    engine.input.mouse.scroll.y += yoffset;
}


static void inputUpdateMouse(void)
{
    // mouse wheel
    engine.input.mouse.scroll.x = 0;
    engine.input.mouse.scroll.y = 0;

    // cursor position
    double x, y;
    glfwGetCursorPos(engine.window, &x, &y);
    engine.input.mouse.cursor.diff.x = x - engine.input.mouse.cursor.pos.x;
    engine.input.mouse.cursor.diff.y = y - engine.input.mouse.cursor.pos.y;
    engine.input.mouse.cursor.pos.x = x;
    engine.input.mouse.cursor.pos.y = y;

    // buttons
    engine.input.mouse.buttons.left = glfwGetMouseButton(engine.window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    engine.input.mouse.buttons.middle = glfwGetMouseButton(engine.window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
    engine.input.mouse.buttons.right = glfwGetMouseButton(engine.window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS;
}
