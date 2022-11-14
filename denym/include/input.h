#ifndef __input_h_
#define __input_h_


#include "denym_common.h"


typedef struct coords2
{
    float x;
    float y;
} coords2;


typedef struct joystick
{
    coords2 axis;
    uint8_t click;
} joystick;

typedef struct input_t
{
    struct
    {
        struct
        {
            coords2 diff;
            coords2 pos;
        } cursor;
        coords2 scroll;
        struct
        {
            uint8_t left;
            uint8_t middle;
            uint8_t right;
        } buttons;
    } mouse;

    // switch pro controller
    struct
    {
        joystick leftStick;
        joystick rightStick;

        struct
        {
            uint8_t l;
            uint8_t zl;
            uint8_t r;
            uint8_t zr;
        } triggers;

        struct
        {
            uint8_t up;
            uint8_t right;
            uint8_t down;
            uint8_t left;
        } dpad;

        struct
        {
            uint8_t a;
            uint8_t b;
            uint8_t x;
            uint8_t y;
            uint8_t plus;
            uint8_t minus;
            uint8_t home;
            uint8_t record;
        } buttons;
    } controller;
} input_t;


int inputCreate(void);

void inputUpdate(void);


#endif
