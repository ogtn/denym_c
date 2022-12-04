#ifndef _light_h_
#define _light_h_


typedef struct dlight_t
{
    float direction[3];
    float intensity;
    float color[3];
    float ambiant;
} dlight_t;


typedef struct plight_t
{
    float position[3];
    float intensity;
    float color[3];
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
