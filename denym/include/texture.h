#ifndef _texture_h_
#define _texture_h_


#include "denym_common.h"


int textureCreate(const char *filename, VkImage *image, VkDeviceMemory *imageMemory);

int createImageRGBA_2D(VkExtent3D imageExtent, VkBuffer src, VkImage *image, VkDeviceMemory *imageMemory);

void imageCopyFromBuffer(VkImage dst, VkBuffer src, VkExtent3D imageExtent);

void imageLayoutTransition(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);

int textureCreateImageView(VkImage image, VkImageView *imageView);

int textureCreateSampler(VkSampler *sampler);

#endif
