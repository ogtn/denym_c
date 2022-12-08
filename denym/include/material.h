#ifndef _material_h_
#define _material_h_


typedef struct color
{
    float r;
    float g;
    float b;
} color;


typedef struct material_t
{
    color color;
    float shininess;
} material_t;


#endif
