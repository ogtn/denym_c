#ifndef _sprite_h
#define _sprite_h


#include "denym_common.h"


typedef struct sprite_t
{
    renderable renderable;
    struct  { float u, v; } spriteCount;
} sprite_t;


sprite spriteCreate(const char *textureName, float sizeU, float sizeV, uint32_t spriteCountU, uint32_t spriteCountV);

void spriteSetSpriteCoordinates(sprite sprite, uint32_t u, uint32_t v);


#endif
