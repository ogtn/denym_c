#ifndef __input_h_
#define __input_h_


#include "denym_common.h"


typedef struct coords2
{
    double x;
    double y;
} coords2;


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
        int buttonLeft;
        struct
        {
            int left;
            int middle;
            int right;
        } buttons;
    } mouse;
} input_t;


int inputCreate(void);

void inputUpdate(void);


#endif
