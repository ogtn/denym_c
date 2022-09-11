#ifndef _texture_h_
#define _texture_h_


#include "denym_common.h"


int textureCreate(const char *filename, VkImage *image, VkDeviceMemory *imageMemory, VkImageView *imageView);

int textureCreateImage2D(uint32_t width, uint32_t height, VkImage *image, VkDeviceMemory *imageMemory);

int textureCreateSampler(VkSampler *sampler);


#endif
