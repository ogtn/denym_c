#ifndef _types_h_
#define _types_h_


typedef struct color
{
    float r;
    float g;
    float b;
} color;


typedef struct vec2f
{
    float u, v;
} vec2f;


typedef struct vec3f
{
    union
    {
        float v[3];
        struct { float x, y, z; };
    };
} vec3f;


#endif
