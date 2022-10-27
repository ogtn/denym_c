#include "texture.h"
#include "image.h"
#include "buffer.h"
#include "core.h"
#include "logger.h"

#include <stb_image.h>
#include <string.h>
#include <math.h>


static const VkFormat TEXTURE_FORMAT = VK_FORMAT_R8G8B8A8_SRGB;


int textureCreate(const char *filename, texture *txtr)
{
    char fullName[FILENAME_MAX];

	snprintf(fullName, FILENAME_MAX, "resources/textures/%s", filename);

    *txtr = resourceCacheGet(engine.caches.textureCache, fullName);

    if(*txtr != NULL)
        return 0;

    int width, height;
    int channelCount;

    stbi_uc *pixels = stbi_load(fullName, &width, &height, &channelCount, STBI_rgb_alpha);
    VkDeviceSize size = (VkDeviceSize)(width * height * STBI_rgb_alpha);

    if(pixels == NULL)
    {
        logWarning("Failed to load texture '%s'", fullName);
        *txtr = resourceCacheGet(engine.caches.textureCache, engine.textureFallback->name);

        return 0;
    }

    // TODO: fix leak when error
    VkBuffer stagingBuffer;
    VkDeviceMemory stagingBufferMemory;

    if(bufferCreate(size, &stagingBuffer, &stagingBufferMemory, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VK_BUFFER_USAGE_TRANSFER_SRC_BIT))
    {
        logError("Failed to create buffer for texture '%s'", fullName);

        return -1;
    }

    void *data;

    if(vkMapMemory(engine.vulkanContext.device, stagingBufferMemory, 0, size, 0, &data))
    {
        logError("Failed to map memory for texture '%s'", fullName);

        return -1;
    }

    memcpy(data, pixels, size);
    vkUnmapMemory(engine.vulkanContext.device, stagingBufferMemory);
    stbi_image_free(pixels);

    texture newTexture = calloc(1, sizeof *newTexture);
    newTexture->extent.width = (uint32_t)width;
    newTexture->extent.height = (uint32_t)height;
    newTexture->extent.depth = 1;
    newTexture->mipLevelCount = (uint32_t)floor(log2(fmax((double)width, (double)(height))));

    textureCreateImage2D(newTexture);
    // transition needed before writing to
    imageLayoutTransition(newTexture->image, newTexture->mipLevelCount, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    imageCopyFromBuffer(newTexture->image, stagingBuffer, newTexture->extent);

    // transition needed before accessing it from shaders
    imageGenerateMipMaps(newTexture->image, width, height, newTexture->mipLevelCount);

    vkDestroyBuffer(engine.vulkanContext.device, stagingBuffer, NULL);
    vkFreeMemory(engine.vulkanContext.device, stagingBufferMemory, NULL);

    *txtr = newTexture;
    strncpy(newTexture->name, fullName, sizeof newTexture->name);
    resourceCacheAdd(engine.caches.textureCache, newTexture->name, newTexture);

    return imageViewCreate2D(newTexture->image, newTexture->mipLevelCount, TEXTURE_FORMAT, &newTexture->imageView);
}


void textureDestroy(texture texture)
{
    VkBool32 needDestruction;

    resourceCacheRemove(engine.caches.textureCache, texture->name, &needDestruction);

    if(needDestruction)
    {
        vkDestroyImageView(engine.vulkanContext.device, texture->imageView, NULL);
        vkDestroyImage(engine.vulkanContext.device, texture->image, NULL);
        vkFreeMemory(engine.vulkanContext.device, texture->imageMemory, NULL);
        free(texture);
    }
}


int textureCreateImage2D(texture texture)
{
    return imageCreate2D(
        texture->extent.width, texture->extent.height,
        texture->mipLevelCount, TEXTURE_FORMAT, VK_SAMPLE_COUNT_1_BIT,
        VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        &texture->image, &texture->imageMemory);
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
    samplerParams.maxLod = VK_LOD_CLAMP_NONE;
    samplerParams.mipLodBias = 0;

    if(vkCreateSampler(engine.vulkanContext.device, &samplerParams, NULL, sampler))
    {
        logError("Failed to create texture sampler");

        return -1;
    }

    return VK_SUCCESS;
}
