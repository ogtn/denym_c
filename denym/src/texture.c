#include "texture.h"
#include "buffer.h"
#include "core.h"

#include <stb_image.h>
#include <string.h>


int textureCreate(void)
{
    char fullName[FILENAME_MAX];

	snprintf(fullName, FILENAME_MAX, "resources/textures/%s", "lena.jpg");

    int width, height;
    int channelCount;

    stbi_uc *pixels = stbi_load(fullName, &width, &height, &channelCount, STBI_rgb_alpha);
    VkDeviceSize size = (VkDeviceSize)(width * height * STBI_rgb_alpha);

    if(pixels == NULL)
    {
        fprintf(stderr, "Failed to load texture %s\n", fullName);

        return -1;
    }

    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    if(createBuffer(size, &stagingBuffer, &stagingBufferMemory, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VK_BUFFER_USAGE_TRANSFER_SRC_BIT))
    {
        fprintf(stderr, "Failed to create buffer for texture %s\n", fullName);

        return -1;
    }

    void *data;

    if(vkMapMemory(engine.vulkanContext.device, stagingBufferMemory, 0, size, 0, &data))
    {
        fprintf(stderr, "Failed to map memory for texture %s\n", fullName);

        return -1;
    }

    memcpy(data, pixels, size);
    vkUnmapMemory(engine.vulkanContext.device, stagingBufferMemory);
    stbi_image_free(pixels);

    VkImage textureImage;
    VkDeviceMemory textureImageMemory;
    VkExtent3D textureExtent = { .width = (uint32_t)width, .height = (uint32_t)height, .depth = 1 };

    createImageRGBA_2D(textureExtent, stagingBuffer, &textureImage, &textureImageMemory);
    vkDestroyBuffer(engine.vulkanContext.device, stagingBuffer, NULL);
    vkFreeMemory(engine.vulkanContext.device, stagingBufferMemory, NULL);

    return 0;
}


int createImageRGBA_2D(VkExtent3D imageExtent, VkBuffer src, VkImage *image, VkDeviceMemory *imageMemory)
{
    VkImageCreateInfo imageCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.extent = imageExtent;
    imageCreateInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 1;

    // best perf
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;

    // we don't care about the initial state because we're writing to it from the buffer
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageCreateInfo.flags = 0;

    if(vkCreateImage(engine.vulkanContext.device, &imageCreateInfo, NULL, image))
    {
        fprintf(stderr, "Failed to create image\n");

        return -1;
    }

    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(engine.vulkanContext.device, *image, &memoryRequirements);

    // TODO check this ?
	// memoryRequirements.alignment;

	VkMemoryAllocateInfo memoryAllocateInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	findMemoryTypeIndex(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryAllocateInfo.memoryTypeIndex);

	if(vkAllocateMemory(engine.vulkanContext.device, &memoryAllocateInfo, NULL, imageMemory))
		return -1;

	// only one image that takes all the memory, so offset is 0
	if(vkBindImageMemory(engine.vulkanContext.device, *image, *imageMemory, 0))
		return -1;

    // transition needed before writing to
    imageLayoutTransition(*image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    imageCopyFromBuffer(*image, src, imageExtent);

    // transition needed before accessing it from shaders
    imageLayoutTransition(*image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

    return VK_SUCCESS;
}


void imageCopyFromBuffer(VkImage dst, VkBuffer src, VkExtent3D imageExtent)
{
    VkBufferImageCopy copyParams;
    copyParams.bufferOffset = 0;
    copyParams.bufferImageHeight = 0;
    copyParams.bufferRowLength = 0;
    copyParams.imageExtent = imageExtent;
    copyParams.imageOffset.x = 0;
    copyParams.imageOffset.y = 0;
    copyParams.imageOffset.z = 0;
    copyParams.imageSubresource.mipLevel = 0;
    copyParams.imageSubresource.layerCount = 1;
    copyParams.imageSubresource.baseArrayLayer = 0;
    copyParams.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;

    VkCommandBuffer commandBuffer;
    initiateCopyCommandBuffer(&commandBuffer);
    vkCmdCopyBufferToImage(commandBuffer, src, dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyParams);
    terminateCopyCommandBuffer(commandBuffer);
}


void imageLayoutTransition(VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout)
{
    VkImageMemoryBarrier barrier = { VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER };
    barrier.image = image;
    barrier.oldLayout = oldLayout;
    barrier.newLayout = newLayout;

    barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
    barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

    barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    barrier.subresourceRange.baseArrayLayer = 0;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.subresourceRange.layerCount = 1;
    barrier.subresourceRange.levelCount = 1;

    VkPipelineStageFlags srcStage, dstStage;

    // Complex shit. For allowed combinations of layout and access, check this :
    // https://registry.khronos.org/vulkan/specs/1.3-extensions/html/chap7.html#synchronization-access-types-supported
    if(oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
    {
        barrier.srcAccessMask = 0;
        barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        srcStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
        dstStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
    }
    else if(oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
    {
        barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
        barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
        srcStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        dstStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    }
    else
    {
        fprintf(stderr, "Unsupported image layout transition\n");

        return;
    }

    VkCommandBuffer commandBuffer;
    initiateCopyCommandBuffer(&commandBuffer);

    vkCmdPipelineBarrier(
        commandBuffer, srcStage, dstStage,
        0,              // check what available flags mean
        0, NULL,        // memory barrier
        0, NULL,        // buffer barrier
        1, &barrier);   // image barrier

    terminateCopyCommandBuffer(commandBuffer);
}

// TODO : in the future, place all copy operations in a single command buffer, to speed things up
