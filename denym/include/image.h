#ifndef _image_h_
#define _image_h_


#include "denym_common.h"


int imageCreateDepth(uint32_t width, uint32_t height, VkFormat format, VkSampleCountFlagBits samples, VkImage *image, VkDeviceMemory *imageMemory);

int imageCreate2D(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkSampleCountFlagBits samples, VkImageUsageFlags usage, VkImage *image, VkDeviceMemory *imageMemory);

void imageCopyFromBuffer(VkImage dst, VkBuffer src, VkExtent3D imageExtent);

void imageLayoutTransition(VkImage image, uint32_t mipLevels, VkImageLayout oldLayout, VkImageLayout newLayout);

void imageGenerateMipMaps(VkImage image, int32_t width, int32_t height, uint32_t mipLevels);

int imageViewCreate2D(VkImage image, uint32_t mipLevels, VkFormat format, VkImageView *imageView);

int imageViewCreateDepth(VkImage image, VkFormat format, VkImageView *imageView);

int imageViewCreate(VkImage image, uint32_t mipLevels, VkFormat format, VkImageAspectFlags aspect, VkImageView *imageView);


#endif
