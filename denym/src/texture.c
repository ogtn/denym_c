#include "texture.h"
#include "buffer.h"
#include "core.h"

#include <stb_image.h>
#include <string.h>


static const VkFormat TEXTURE_FORMAT = VK_FORMAT_R8G8B8A8_SRGB;


int textureCreate(const char *filename, VkImage *image, VkDeviceMemory *imageMemory, VkImageView *imageView)
{
    char fullName[FILENAME_MAX];

	snprintf(fullName, FILENAME_MAX, "resources/textures/%s", filename);

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


    VkExtent3D textureExtent = { .width = (uint32_t)width, .height = (uint32_t)height, .depth = 1 };

    createImageTexture2D(textureExtent.width, textureExtent.height, image, imageMemory);
    // transition needed before writing to
    imageLayoutTransition(*image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    imageCopyFromBuffer(*image, stagingBuffer, textureExtent);

    // transition needed before accessing it from shaders
    imageLayoutTransition(*image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkDestroyBuffer(engine.vulkanContext.device, stagingBuffer, NULL);
    vkFreeMemory(engine.vulkanContext.device, stagingBufferMemory, NULL);

    return createImageView2D(*image, TEXTURE_FORMAT, imageView);
}


int createImageTexture2D(uint32_t width, uint32_t height, VkImage *image, VkDeviceMemory *imageMemory)
{
    return createImage2D(
        width, height, TEXTURE_FORMAT,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        image, imageMemory);
}


int createImageDepth(uint32_t width, uint32_t height, VkFormat format, VkImage *image, VkDeviceMemory *imageMemory)
{
    return createImage2D(
        width, height, format,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        image, imageMemory);
}


int createImage2D(uint32_t width, uint32_t height, VkFormat format, VkImageUsageFlags usage, VkImage *image, VkDeviceMemory *imageMemory)
{
    VkImageCreateInfo imageCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.extent.width = width;
    imageCreateInfo.extent.height = height;
    imageCreateInfo.extent.depth = 1;
    imageCreateInfo.format = format;
    imageCreateInfo.mipLevels = 1;
    imageCreateInfo.arrayLayers = 1;

    // best perf
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;

    // we don't care about the initial state because we're writing to it from the buffer
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.usage = usage;
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


int createImageView2D(VkImage image, VkFormat format, VkImageView *imageView)
{
    return createImageView(image, format, VK_IMAGE_ASPECT_COLOR_BIT, imageView);
}


int createImageViewDepth(VkImage image, VkFormat format, VkImageView *imageView)
{
    return createImageView(image, format, VK_IMAGE_ASPECT_DEPTH_BIT, imageView);
}


int createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspect, VkImageView *imageView)
{
	VkImageViewCreateInfo createInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	createInfo.image = image;
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.format = format;
	createInfo.subresourceRange.aspectMask = aspect;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.levelCount = 1;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount = 1;

	if (vkCreateImageView(engine.vulkanContext.device, &createInfo, NULL, imageView))
	{
		fprintf(stderr, "Failed to create image view.\n");

		return -1;
	}

	return VK_SUCCESS;
}


int textureCreateSampler(VkSampler *sampler)
{
    VkSamplerCreateInfo samplerParams = { VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO };
    samplerParams.minFilter = VK_FILTER_LINEAR;
    samplerParams.magFilter = VK_FILTER_LINEAR;

    samplerParams.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerParams.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerParams.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK; // used with VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER

    samplerParams.anisotropyEnable = engine.vulkanContext.physicalDeviceFeatures.samplerAnisotropy;
    samplerParams.maxAnisotropy = engine.vulkanContext.physicalDeviceProperties.limits.maxSamplerAnisotropy;

    samplerParams.unnormalizedCoordinates = VK_FALSE; // whe true, texture coordinates in pixels instead of [0;1]

    // TODO Usefull for shadowmapping, see https://developer.nvidia.com/gpugems/GPUGems/gpugems_ch11.html
    samplerParams.compareEnable = VK_FALSE;
    samplerParams.compareOp = VK_COMPARE_OP_ALWAYS;

    // mipmapping
    samplerParams.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerParams.minLod = 0;
    samplerParams.maxLod = 0;
    samplerParams.mipLodBias = 0;

    if(vkCreateSampler(engine.vulkanContext.device, &samplerParams, NULL, sampler))
    {
        fprintf(stderr, "Failed to create texture sampler\n");

        return -1;
    }

    return VK_SUCCESS;
}

// TODO : in the future, place all copy operations in a single command buffer, to speed things up
