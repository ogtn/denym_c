#include "input_internal.h"
#include "core.h"
#include "logger.h"

#include <string.h>


static const float defaultJoystickDeadZone = 0.1f;


static void inputGlfwKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);

static void inputGlfwScrollCallback(GLFWwindow* window, double xoffset, double yoffset);

static void inputGlfwJoystickCallback(int id, int event);

static void inputUpdateMouse(void);

static void inputUpdateController(void);

static float removeJoystickDeadZone(float value, float threshold);


int inputCreate(void)
{
    memset(&engine.input, 0, sizeof engine.input);

    glfwSetKeyCallback(engine.window, inputGlfwKeyCallback);
    glfwSetScrollCallback(engine.window, inputGlfwScrollCallback);
    glfwSetJoystickCallback(inputGlfwJoystickCallback);

    if(engine.settings.captureMouse)
    {
        glfwSetInputMode(engine.window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

        if(glfwRawMouseMotionSupported())
            glfwSetInputMode(engine.window, GLFW_RAW_MOUSE_MOTION, GLFW_TRUE);
    }

    return 0;
}


void inputUpdate(input input)
{
    inputUpdateMouse();
    inputUpdateController();
    glfwPollEvents();

    if(input)
        memcpy(input, &engine.input, sizeof *input);
}


int inputIsKeyPressed(inputKeyId key)
{
    return glfwGetKey(engine.window, key) == GLFW_PRESS;
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
    engine.input.mouse.scroll.x += (float)xoffset;
    engine.input.mouse.scroll.y += (float)yoffset;
}


static void inputGlfwJoystickCallback(int id, int event)
{
    if(event == GLFW_CONNECTED)
    {
        logWarning("Controller \"%s\" with id %d has been connected",
            glfwGetJoystickName(id), id);
    }
    else
    {
        logWarning("Controller with id %d has been disconnected", id);
    }
}


static void inputUpdateMouse(void)
{
    // mouse wheel
    engine.input.mouse.scroll.x = 0;
    engine.input.mouse.scroll.y = 0;

    // cursor position
    double x, y;
    glfwGetCursorPos(engine.window, &x, &y);
    engine.input.mouse.cursor.diff.x = (float)x - engine.input.mouse.cursor.pos.x;
    engine.input.mouse.cursor.diff.y = (float)y - engine.input.mouse.cursor.pos.y;
    engine.input.mouse.cursor.pos.x = (float)x;
    engine.input.mouse.cursor.pos.y = (float)y;

    // buttons
    engine.input.mouse.buttons.left = glfwGetMouseButton(engine.window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS;
    engine.input.mouse.buttons.middle = glfwGetMouseButton(engine.window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS;
    engine.input.mouse.buttons.right = glfwGetMouseButton(engine.window, GLFW_MOUSE_BUTTON_MIDDLE) == GLFW_PRESS;
    engine.input.mouse.buttons.next = glfwGetMouseButton(engine.window, GLFW_MOUSE_BUTTON_4) == GLFW_PRESS;
    engine.input.mouse.buttons.previous = glfwGetMouseButton(engine.window, GLFW_MOUSE_BUTTON_5) == GLFW_PRESS;
}


static void inputUpdateController(void)
{
    if(glfwJoystickPresent(0))
    {
        int count;
        const float *axes = glfwGetJoystickAxes(0, &count);

        engine.input.controller.leftStick.axis.x = removeJoystickDeadZone(axes[0], defaultJoystickDeadZone);
        engine.input.controller.leftStick.axis.y = removeJoystickDeadZone(axes[1], defaultJoystickDeadZone);
        engine.input.controller.rightStick.axis.x = removeJoystickDeadZone(axes[2], defaultJoystickDeadZone);
        engine.input.controller.rightStick.axis.y = removeJoystickDeadZone(axes[3], defaultJoystickDeadZone);

        const unsigned char *buttons = glfwGetJoystickButtons(0, &count);

        engine.input.controller.buttons.b = buttons[0];
        engine.input.controller.buttons.a = buttons[1];
        engine.input.controller.buttons.y = buttons[2];
        engine.input.controller.buttons.x = buttons[3];

        engine.input.controller.triggers.l = buttons[4];
        engine.input.controller.triggers.r = buttons[5];
        engine.input.controller.triggers.zl = buttons[6];
        engine.input.controller.triggers.zr = buttons[7];

        engine.input.controller.buttons.plus = buttons[8];
        engine.input.controller.buttons.minus = buttons[9];

        engine.input.controller.leftStick.click = buttons[10];
        engine.input.controller.rightStick.click = buttons[11];

        engine.input.controller.buttons.home = buttons[12];
        engine.input.controller.buttons.record = buttons[13];

        engine.input.controller.dpad.up = buttons[18];
        engine.input.controller.dpad.right = buttons[19];
        engine.input.controller.dpad.down = buttons[20];
        engine.input.controller.dpad.left = buttons[21];

        /*
        char mapping[32] = { '\0' };

        for(int i = 0; i < 22; i++)
            mapping[i] = buttons[i] + '0';

        logWarning("%s state : [%s] (%.3f;%.3f) => (%.3f;%.3f)    (%.3f;%.3f) => (%.3f;%.3f)",
            glfwGetJoystickName(0), mapping,
            axes[0], axes[1], engine.input.controller.leftStick.axis.x, engine.input.controller.leftStick.axis.y,
            axes[2], axes[3], engine.input.controller.rightStick.axis.x, engine.input.controller.rightStick.axis.y);
        */
    }
}


static float removeJoystickDeadZone(float value, float threshold)
{
    if((value > 0 && value < threshold) || (value < 0 && value > -threshold))
        return 0;

    return value;
}
