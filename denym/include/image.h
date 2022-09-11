#ifndef _image_h_
#define _image_h_


#include "denym_common.h"


int createImageDepth(uint32_t width, uint32_t height, VkFormat format, VkImage *image, VkDeviceMemory *imageMemory);

int createImage2D(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, VkImage *image, VkDeviceMemory *imageMemory);

void imageCopyFromBuffer(VkImage dst, VkBuffer src, VkExtent3D imageExtent);

void imageLayoutTransition(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);

int createImageView2D(VkImage image, VkFormat format, VkImageView *imageView);

int createImageViewDepth(VkImage image, VkFormat format, VkImageView *imageView);

int createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspect, VkImageView *imageView);


#endif
