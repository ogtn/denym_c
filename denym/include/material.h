#ifndef _material_h_
#define _material_h_


#include "denym_common.h"


typedef struct color
{
    float r;
    float g;
    float b;
} color;


typedef struct material
{
    color diffuse;
    color ambient;
    color specular;
    color emissive;
    float shininess;
} material;


#endif
