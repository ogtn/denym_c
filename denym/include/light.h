#ifndef _light_h_
#define _light_h_


#include "types.h"


typedef struct dlight_t
{
    vec3f direction;
    float intensity;
    color color;
    float ambiant;
} dlight_t;


typedef struct plight_t
{
    vec3f position;
    float intensity;
    color color;
    float ambiant;
    float constantAttenuation;
    float linearAttenuation;
    float quadraticAttenuation;
    float padding;
} plight_t;


typedef struct dlight_t *dlight;
typedef struct plight_t *plight;


void dlightInit(dlight light);

void plightInit(plight light);


#endif
