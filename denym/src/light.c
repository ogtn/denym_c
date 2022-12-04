#include "light.h"

#include <string.h>


void dlightInit(dlight light)
{
    memset(light, 0, sizeof *light);

    light->direction[0] = light->direction[1] = light->direction[2] = 1;
    light->intensity = 0.01f;
    light->color[0] = light->color[1] = light->color[2] = 1;
    light->ambiant = 0.008f;
}


void plightInit(plight light)
{
    memset(light, 0, sizeof *light);

    light->intensity = 2.f;
    light->color[0] = light->color[1] = light->color[2] = 1;
    light->ambiant = 0.008f;
    light->constantAttenuation = 1;
    light->linearAttenuation = 0.5f;
    light->quadraticAttenuation = 2.f;
}
