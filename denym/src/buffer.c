#include "buffer.h"
#include "core.h"
#include "logger.h"

#include <stdio.h>
#include <string.h>


int bufferCreateVertex(VkDeviceSize size, VkBuffer* buffer, VkDeviceMemory* vertexBufferMemory, void* src)
{
	bufferCreate(size, buffer, vertexBufferMemory, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

	// copy the data to the allocated memory of the vertex buffer
	void *dest;
	vkMapMemory(engine.vulkanContext.device, *vertexBufferMemory, 0, size, 0, &dest);
	memcpy(dest, src, size);
	vkUnmapMemory(engine.vulkanContext.device, *vertexBufferMemory);

	return 0;
}


int bufferCreateVertexWithStaging(VkDeviceSize size, VkBuffer* buffer, VkDeviceMemory* bufferMemory, void* src)
{
	return bufferCreateWithStaging(size, buffer, bufferMemory, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, src);
}


int bufferCreateIndexWithStaging(VkDeviceSize size, VkBuffer* buffer, VkDeviceMemory* bufferMemory, void* src)
{
	return bufferCreateWithStaging(size, buffer, bufferMemory, VK_BUFFER_USAGE_INDEX_BUFFER_BIT, src);
}


int bufferCreateWithStaging(VkDeviceSize size, VkBuffer* buffer, VkDeviceMemory* bufferMemory, VkBufferUsageFlags bufferUsage, void* src)
{
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	// staging buffer, accessible from the CPU, and usable as a src for buffer transfert
	bufferCreate(size, &stagingBuffer, &stagingBufferMemory, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

	// copy the data to the allocated memory of the staging buffer
	void *dest;
	vkMapMemory(engine.vulkanContext.device, stagingBufferMemory, 0, size, 0, &dest);
	memcpy(dest, src, size);
	vkUnmapMemory(engine.vulkanContext.device, stagingBufferMemory);

	// buffer on device side, faster, and usable as a dst for buffer transfert
	bufferCreate(size, buffer, bufferMemory, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, bufferUsage | VK_BUFFER_USAGE_TRANSFER_DST_BIT);
	bufferCopy(stagingBuffer, *buffer, size);
	vkDestroyBuffer(engine.vulkanContext.device, stagingBuffer, NULL);
	vkFreeMemory(engine.vulkanContext.device, stagingBufferMemory, NULL);

	return 0;
}


int bufferCreate(VkDeviceSize size, VkBuffer *buffer, VkDeviceMemory *bufferMemory, VkMemoryPropertyFlags properties, VkBufferUsageFlags bufferUsage)
{
	VkBufferCreateInfo bufferCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufferCreateInfo.size = size;
	bufferCreateInfo.usage = bufferUsage;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	if(vkCreateBuffer(engine.vulkanContext.device, &bufferCreateInfo, NULL, buffer))
		return -1;

	VkMemoryRequirements bufferMemoryRequirements;
	vkGetBufferMemoryRequirements(engine.vulkanContext.device, *buffer, &bufferMemoryRequirements);

    // TODO check this ?
	// bufferMemoryRequirements.alignment;

	VkMemoryAllocateInfo memoryAllocateInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
	memoryAllocateInfo.allocationSize = bufferMemoryRequirements.size;
	bufferFindMemoryTypeIndex(bufferMemoryRequirements.memoryTypeBits, properties, &memoryAllocateInfo.memoryTypeIndex);

	if(vkAllocateMemory(engine.vulkanContext.device, &memoryAllocateInfo, NULL, bufferMemory))
		return -1;

	// only one buffer that takes all the memory, so offset is 0
	if(vkBindBufferMemory(engine.vulkanContext.device, *buffer, *bufferMemory, 0))
		return -1;

	// TODO: maybe overload some vk functions and use this to inject __LINE__ __FILE__ ?
#ifdef _DEBUG
	static int cpt;
	VkDebugUtilsObjectNameInfoEXT objectNameInfo = { VK_STRUCTURE_TYPE_DEBUG_UTILS_OBJECT_NAME_INFO_EXT };
	objectNameInfo.objectType = VK_OBJECT_TYPE_BUFFER;

	char name[16];
	snprintf(name, sizeof name, "buffer_%d", cpt++);
	logInfo("%s", name);
	objectNameInfo.pObjectName = name;
	objectNameInfo.objectHandle = (uint64_t)*buffer;
	engine.vulkanContext.SetDebugUtilsObjectNameEXT(engine.vulkanContext.device, &objectNameInfo);
#endif

	return 0;
}


int bufferCopy(VkBuffer src, VkBuffer dst, VkDeviceSize size)
{
	VkCommandBuffer commandBuffer;
	VkBufferCopy copyRegion;
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;

	bufferInitiateCopyCmdBuffer(&commandBuffer);
	vkCmdCopyBuffer(commandBuffer, src, dst, 1, &copyRegion);
	bufferTerminateCopyCmdBuffer(commandBuffer);

	return VK_SUCCESS;
}


int bufferInitiateCopyCmdBuffer(VkCommandBuffer *commandBuffer)
{
	VkCommandBufferAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	allocInfo.commandPool = engine.vulkanContext.bufferCopyCommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	VkResult result = vkAllocateCommandBuffers(engine.vulkanContext.device, &allocInfo, commandBuffer);

	if (result != VK_SUCCESS)
	{
		logError("Failed to allocate command buffer.\n");

		return result;
	}

	VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(*commandBuffer, &beginInfo);

	return VK_SUCCESS;
}


void bufferTerminateCopyCmdBuffer(VkCommandBuffer commandBuffer)
{
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(engine.vulkanContext.graphicQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(engine.vulkanContext.graphicQueue); // here we just wait, but could do multiple operations in // and use a fence to wait them all
	vkFreeCommandBuffers(engine.vulkanContext.device, engine.vulkanContext.bufferCopyCommandPool, 1, &commandBuffer);
}


int bufferFindMemoryTypeIndex(uint32_t typeFilter, VkMemoryPropertyFlags properties, uint32_t* index)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(engine.vulkanContext.physicalDevice, &memProperties);

	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
		{
			*index = i;

			return 0;
		}
	}

	return -1;
}
