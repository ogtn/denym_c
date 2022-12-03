#include "light.h"


dlight dlightCreate(void)
{
    dlight light = calloc(1, sizeof *light);

    light->direction[0] = 1;
    light->direction[1] = -1;
    light->direction[2] = 0.25;

    light->intensity = 0.5f;

    light->color[0] = 255.f / 255.f;
    light->color[1] = 234.f / 255.f;
    light->color[2] = 173.f / 255.f;

    light->ambiant = 0.008f;

    return light;
}
