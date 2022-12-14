#include "sprite.h"
#include "renderable.h"
#include "logger.h"


sprite spriteCreate(const char *textureName, float sizeU, float sizeV, uint32_t spriteCountU, uint32_t spriteCountV)
{
    sprite sprite = calloc(1, sizeof *sprite);

	float positions[] =
	{
		0, 0,
		sizeU, 0,
		0, sizeV,

		sizeU, 0,
        sizeU, sizeV,
		0, sizeV
	};

    geometryParams geometryParams = geometryCreateParameters(6, 0);
    geometryParamsAddPositions2D(geometryParams, positions);

	renderableCreateParams params = {
		.sendMVP = 1,
		.compactMVP = 1,
		.textureName = textureName,
		.useNearestSampler = 1,
		.vertShaderName = "sprite.vert.spv",
		.fragShaderName = "sprite.frag.spv",
		.pushConstantSize = sizeof(float) * 6,
		.geometry = geometryCreate(geometryParams)
	};

	sprite->renderable = renderableCreate(&params, 1);
	sprite->spriteCount.u = spriteCountU;
	sprite->spriteCount.v = spriteCountV;

    return sprite;
}


void spriteSetSpriteCoordinates(sprite sprite, uint32_t u, uint32_t v)
{
	float start_u = 1 / sprite->spriteCount.u * u;
	float end_u = 1 / sprite->spriteCount.u * (u + 1);
	float start_v = 1 / sprite->spriteCount.v * v;
	float end_v = 1 / sprite->spriteCount.v * (v + 1);

	float coords[] = { start_u, start_v, end_u, end_v };

	if(sprite->flipVertical)
	{
		float tmp = coords[0];
		coords[0] = coords[2];
		coords[2] = tmp;
	}

	if(sprite->flipHorizontal)
	{
		float tmp = coords[1];
		coords[1] = coords[3];
		coords[3] = tmp;
	}

	renderableUpdatePushConstant(sprite->renderable, &coords);
}


void spriteSetPosition(sprite sprite, float x, float y)
{
	renderableSetPosition(sprite->renderable, x, y, 0);
}


void spriteFlip(sprite sprite, int verticalAxis, int horizontalAxis)
{
	sprite->flipVertical = verticalAxis;
	sprite->flipHorizontal = horizontalAxis;
}
