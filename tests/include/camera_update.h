#include "denym.h"


static void updateCameraPosition(const input input, camera camera, float duration)
{
    float speed = duration * 5;
    float speed_x = 0;
    float speed_y = 0;
    float speed_z = 0;

    if(inputIsKeyPressed(INPUT_KEY_W))
        speed_x += speed;
    if(inputIsKeyPressed(INPUT_KEY_S))
        speed_x -= speed;
    if(inputIsKeyPressed(INPUT_KEY_D))
        speed_y += speed;
    if(inputIsKeyPressed(INPUT_KEY_A))
        speed_y -= speed;
    if(inputIsKeyPressed(INPUT_KEY_SPACE))
        speed_z += speed;
    if(inputIsKeyPressed(INPUT_KEY_LEFT_CONTROL))
        speed_z -= speed;

    if(input->controller.isPresent)
    {
        if(input->controller.triggers.zl)
            speed = duration * 15;
        else
            speed = duration * 5;

        speed_x = -input->controller.leftStick.axis.y * speed;
        speed_y = input->controller.leftStick.axis.x * speed;

        if(input->controller.buttons.x)
            speed_z += speed;
        if(input->controller.buttons.b)
            speed_z -= speed;
    }

    cameraMove(camera, speed_x, speed_y, speed_z);
}


static void updateCameraRotation(const input input, camera camera, float duration)
{
    float yaw = 0;
    float pitch = 0;

    if(input->controller.isPresent)
    {
        float angularSpeed = duration * 2.5f;
        yaw = input->controller.rightStick.axis.x * angularSpeed;
        pitch = -input->controller.rightStick.axis.y * angularSpeed;
    }

    if(input->mouse.buttons.left)
    {
        float angularSpeed = duration * 0.1f;
        yaw = input->mouse.cursor.diff.x * angularSpeed;
        pitch = input->mouse.cursor.diff.y * angularSpeed;
    }

    cameraRotate(camera, yaw, pitch, 0);
}


static void updateCameraFov(const input input, camera camera)
{
    if(input->controller.isPresent)
    {
        if(input->controller.triggers.l)
            cameraSetFov(camera, 60);
        else
            cameraSetFov(camera, 90);
    }
    else if(input->mouse.scroll.y != 0)
        cameraFov(camera, -input->mouse.scroll.y);
}


static void updateCameraZoom(const input input, camera camera, float duration)
{
    float zoom = -input->mouse.scroll.y;

    zoom += duration * input->controller.buttons.plus;
    zoom -= duration * input->controller.buttons.minus;

    if(zoom)
        cameraZoom(camera, zoom);
}


static void updateCameraPerspective(input input, camera camera)
{
    float duration = denymGetTimeSinceLastFrame();

    updateCameraPosition(input, camera, duration);
    updateCameraRotation(input, camera, duration);
    updateCameraFov(input, camera);
}


static void updateCameraOrtho(input input, camera camera)
{
    float duration = denymGetTimeSinceLastFrame();

    updateCameraPosition(input, camera, duration);
    updateCameraRotation(input, camera, duration);
    updateCameraZoom(input, camera, duration);
}
