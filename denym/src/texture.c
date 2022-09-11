#include "texture.h"
#include "image.h"
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

    textureCreateImage2D(textureExtent.width, textureExtent.height, image, imageMemory);
    // transition needed before writing to
    imageLayoutTransition(*image, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    imageCopyFromBuffer(*image, stagingBuffer, textureExtent);

    // transition needed before accessing it from shaders
    imageLayoutTransition(*image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    vkDestroyBuffer(engine.vulkanContext.device, stagingBuffer, NULL);
    vkFreeMemory(engine.vulkanContext.device, stagingBufferMemory, NULL);

    return createImageView2D(*image, TEXTURE_FORMAT, imageView);
}


int textureCreateImage2D(uint32_t width, uint32_t height, VkImage *image, VkDeviceMemory *imageMemory)
{
    return createImage2D(
        width, height, TEXTURE_FORMAT,
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        image, imageMemory);
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
