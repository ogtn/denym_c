#include "light.h"

#include <string.h>


void dlightInit(dlight light)
{
    memset(light, 0, sizeof *light);

    light->direction.x = light->direction.y = light->direction.z = 1;
    light->intensity = 0.01f;
    light->color.r = light->color.g = light->color.b = 1;
    light->ambiant = 0.008f;
}


void plightInit(plight light)
{
    memset(light, 0, sizeof *light);

    light->intensity = 2.f;
    light->color.r = light->color.g = light->color.b = 1;
    light->ambiant = 0.008f;
    light->constantAttenuation = 1;
    light->linearAttenuation = 0.5f;
    light->quadraticAttenuation = 2.f;
}
