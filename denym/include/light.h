#ifndef _light_h_
#define _light_h_


#include "denym_common.h"


typedef struct light_t
{
    vec3 direction;
    float intensity;
    vec3 color;
    float ambiant;
} light_t;


light lightCreate(void);


#endif
