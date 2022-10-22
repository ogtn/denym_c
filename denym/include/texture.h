#ifndef _texture_h_
#define _texture_h_


#include "denym_common.h"


typedef struct texture_t
{
	char name[FILENAME_MAX];
	VkImage image;
	VkDeviceMemory imageMemory;
	VkImageView imageView;
	VkExtent3D extent;
	uint32_t mipLevelCount;
} texture_t;


int textureCreate(const char *filename, texture *texture);

void textureDestroy(texture texture);

int textureCreateImage2D(texture texture);

int textureCreateSampler(VkSampler *sampler);


#endif
