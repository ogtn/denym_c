#ifndef _material_h_
#define _material_h_


#include "types.h"


typedef struct material_t
{
    color color;
    float shininess;
} material_t;

typedef struct material_t *material;


#endif
