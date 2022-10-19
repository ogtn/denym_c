#include "image.h"
#include "buffer.h"
#include "core.h"
#include "logger.h"


int imageCreateDepth(uint32_t width, uint32_t height, VkFormat format, VkSampleCountFlagBits samples, VkImage *image, VkDeviceMemory *imageMemory)
{
    return imageCreate2D(
        width, height, 1, format, samples,
        VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        image, imageMemory);
}


int imageCreate2D(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkSampleCountFlagBits samples, VkImageUsageFlags usage, VkImage *image, VkDeviceMemory *imageMemory)
{
    VkImageCreateInfo imageCreateInfo = { VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO };
    imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
    imageCreateInfo.extent.width = width;
    imageCreateInfo.extent.height = height;
    imageCreateInfo.extent.depth = 1;
    imageCreateInfo.format = format;
    imageCreateInfo.mipLevels = mipLevels;
    imageCreateInfo.arrayLayers = 1;

    // best perf
    imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;

    // we don't care about the initial state because we're writing to it from the buffer
    imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    imageCreateInfo.usage = usage;
    imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    imageCreateInfo.samples = samples;
    imageCreateInfo.flags = 0;

    if(vkCreateImage(engine.vulkanContext.device, &imageCreateInfo, NULL, image))
    {
        logError("Failed to create image");

        return -1;
    }

    VkMemoryRequirements memoryRequirements;
    vkGetImageMemoryRequirements(engine.vulkanContext.device, *image, &memoryRequirements);

    // TODO check this ?
	// memoryRequirements.alignment;

	VkMemoryAllocateInfo memoryAllocateInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
	memoryAllocateInfo.allocationSize = memoryRequirements.size;
	bufferFindMemoryTypeIndex(memoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, &memoryAllocateInfo.memoryTypeIndex);

	if(vkAllocateMemory(engine.vulkanContext.device, &memoryAllocateInfo, NULL, imageMemory))
		return -1;

	// only one image that takes all the memory, so offset is 0
	if(vkBindImageMemory(engine.vulkanContext.device, *image, *imageMemory, 0))
		return -1;

    return VK_SUCCESS;
}


void imageCopyFromBuffer(VkImage dst, VkBuffer src, VkExtent3D imageExtent)
{
    VkBufferImageCopy copyParams = {
        .bufferOffset = 0,
        .bufferImageHeight = 0,
        .bufferRowLength = 0,
        .imageExtent = imageExtent,
        .imageOffset.x = 0,
        .imageOffset.y = 0,
        .imageOffset.z = 0,
        .imageSubresource.mipLevel = 0,
        .imageSubresource.layerCount = 1,
        .imageSubresource.baseArrayLayer = 0,
        .imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT
    };

    VkCommandBuffer commandBuffer;
    bufferInitiateCopyCmdBuffer(&commandBuffer);
    vkCmdCopyBufferToImage(commandBuffer, src, dst, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyParams);
    bufferTerminateCopyCmdBuffer(commandBuffer);
}


void imageLayoutTransition(VkImage image, uint32_t mipLevels, VkImageLayout oldLayout, VkImageLayout newLayout)
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
    barrier.subresourceRange.levelCount = mipLevels;

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
        logError("Unsupported image layout transition");

        return;
    }

    VkCommandBuffer commandBuffer;
    bufferInitiateCopyCmdBuffer(&commandBuffer);

    vkCmdPipelineBarrier(
        commandBuffer, srcStage, dstStage,
        0,              // TODO: check what available flags mean
        0, NULL,        // memory barrier
        0, NULL,        // buffer barrier
        1, &barrier);   // image barrier

    bufferTerminateCopyCmdBuffer(commandBuffer);
}


void imageGenerateMipMaps(VkImage image, int32_t width, int32_t height, uint32_t mipLevels)
{
    VkImageMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .image = image,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
        .srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
        .dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
        .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .subresourceRange.levelCount = 1, // one mipmap level at a time
        .subresourceRange.layerCount = 1,
    };

    VkCommandBuffer commandBuffer;
    bufferInitiateCopyCmdBuffer(&commandBuffer);

    int32_t mip_width = width;
    int32_t mip_height = height;

    // TODO : try to do the same work with a single blit operation with mipLevels regions
    for(uint32_t i = 1; i < mipLevels; i++)
    {
        // source level has to be transitionned to become blit source
        // previously it was set as destination for imageCopyFromBuffer()
        barrier.subresourceRange.baseMipLevel = i - 1;

        vkCmdPipelineBarrier(
            commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
            0,
            0, NULL,        // memory barrier
            0, NULL,        // buffer barrier
            1, &barrier);   // image barrier

        VkImageBlit region = {
            .srcOffsets[1].x = mip_width,
            .srcOffsets[1].y = mip_height,
            .srcOffsets[1].z = 1,
            .srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .srcSubresource.mipLevel = i - 1,
            .srcSubresource.layerCount = 1,

            .dstOffsets[1].x = mip_width > 1 ? mip_width / 2 : 1,
            .dstOffsets[1].y = mip_height > 1 ? mip_height / 2 : 1,
            .dstOffsets[1].z = 1,
            .dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .dstSubresource.mipLevel = i,
            .dstSubresource.layerCount = 1
        };

        vkCmdBlitImage(commandBuffer,
            image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
            image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1, &region, VK_FILTER_LINEAR);

        mip_width = region.dstOffsets[1].x;
        mip_height = region.dstOffsets[1].y;
    }

    // transition last level...
    barrier.subresourceRange.baseMipLevel = mipLevels - 1;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
    barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

    vkCmdPipelineBarrier(
        commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
        0,
        0, NULL,        // memory barrier
        0, NULL,        // buffer barrier
        1, &barrier);   // image barrier

    // then transition all levels to shader layout
    barrier.subresourceRange.levelCount = mipLevels;
    barrier.subresourceRange.baseMipLevel = 0;
    barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
    barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
    barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    vkCmdPipelineBarrier(
            commandBuffer, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
            0,
            0, NULL,        // memory barrier
            0, NULL,        // buffer barrier
            1, &barrier);   // image barrier

    bufferTerminateCopyCmdBuffer(commandBuffer);
}


int imageViewCreate2D(VkImage image, uint32_t mipLevels, VkFormat format, VkImageView *imageView)
{
    return imageViewCreate(image, mipLevels, format, VK_IMAGE_ASPECT_COLOR_BIT, imageView);
}


int imageViewCreateDepth(VkImage image, VkFormat format, VkImageView *imageView)
{
    return imageViewCreate(image, 1, format, VK_IMAGE_ASPECT_DEPTH_BIT, imageView);
}


int imageViewCreate(VkImage image, uint32_t mipLevels, VkFormat format, VkImageAspectFlags aspect, VkImageView *imageView)
{
	VkImageViewCreateInfo createInfo = { VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO };
	createInfo.image = image;
	createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
	createInfo.format = format;
	createInfo.subresourceRange.aspectMask = aspect;
	createInfo.subresourceRange.baseMipLevel = 0;
	createInfo.subresourceRange.levelCount = mipLevels;
	createInfo.subresourceRange.baseArrayLayer = 0;
	createInfo.subresourceRange.layerCount = 1;

	if (vkCreateImageView(engine.vulkanContext.device, &createInfo, NULL, imageView))
	{
		logError("Failed to create image view.");

		return -1;
	}

	return VK_SUCCESS;
}

// TODO : in the future, place all copy operations in a single command buffer, to speed things up
