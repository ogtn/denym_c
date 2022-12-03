#ifndef _light_h_
#define _light_h_


#include "denym_common.h"


typedef struct dlight_t
{
    vec3 direction;
    float intensity;
    vec3 color;
    float ambiant;
} dlight_t;


typedef struct plight_t
{
    vec3 position;
    float intensity;
    vec3 color;
    float ambiant;
    float constantAttenuation;
    float linearAttenuation;
    float quadraticAttenuation;
} plight_t;


dlight dlightCreate(void);

plight plightCreate(void);


#endif

