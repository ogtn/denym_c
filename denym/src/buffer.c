#include "buffer.h"
#include "core.h"

#include <stdio.h>
#include <string.h>


int createVertexBuffer(VkDeviceSize size, VkBuffer* buffer, VkDeviceMemory* vertexBufferMemory, void* src)
{
	createBuffer(size, buffer, vertexBufferMemory, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);

	// copy the data to the allocated memory of the vertex buffer
	void *dest;
	vkMapMemory(engine.vulkanContext.device, *vertexBufferMemory, 0, size, 0, &dest);
	memcpy(dest, src, size);
	vkUnmapMemory(engine.vulkanContext.device, *vertexBufferMemory);

	return 0;
}


int createVertexBufferWithStaging(VkDeviceSize size, VkBuffer* buffer, VkDeviceMemory* vertexBufferMemory, void* src)
{
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	// staging buffer, accessible from the CPU, and usable as a src for buffer transfert
	createBuffer(size, &stagingBuffer, &stagingBufferMemory, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

	// copy the data to the allocated memory of the staging buffer
	void *dest;
	vkMapMemory(engine.vulkanContext.device, stagingBufferMemory, 0, size, 0, &dest);
	memcpy(dest, src, size);
	vkUnmapMemory(engine.vulkanContext.device, stagingBufferMemory);

	// vertex buffer on device side, faster, and usable as a dst for buffer transfert
	createBuffer(size, buffer, vertexBufferMemory, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

	copyBuffer(stagingBuffer, *buffer, size);

	return 0;
}


// TODO factorize this with createVertexBufferWithStaging, only difference is flag VK_BUFFER_USAGE_INDEX_BUFFER_BIT instead of VK_BUFFER_USAGE_VERTEX_BUFFER_BIT
int createIndexBufferWithStaging(VkDeviceSize size, VkBuffer* buffer, VkDeviceMemory* vertexBufferMemory, void* src)
{
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingBufferMemory;

	// staging buffer, accessible from the CPU, and usable as a src for buffer transfert
	createBuffer(size, &stagingBuffer, &stagingBufferMemory, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);

	// copy the data to the allocated memory of the staging buffer
	void *dest;
	vkMapMemory(engine.vulkanContext.device, stagingBufferMemory, 0, size, 0, &dest);
	memcpy(dest, src, size);
	vkUnmapMemory(engine.vulkanContext.device, stagingBufferMemory);

	// index buffer on device side, faster, and usable as a dst for buffer transfert
	createBuffer(size, buffer, vertexBufferMemory, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT);

	copyBuffer(stagingBuffer, *buffer, size);

	return 0;
}


int createBuffer(VkDeviceSize size, VkBuffer *buffer, VkDeviceMemory *vertexBufferMemory, VkMemoryPropertyFlags properties, VkBufferUsageFlags bufferUsage)
{
	VkBufferCreateInfo bufferCreateInfo = { VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO };
	bufferCreateInfo.size = size;
	bufferCreateInfo.usage = bufferUsage;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

	// TODO destroy this with geometry
	if(vkCreateBuffer(engine.vulkanContext.device, &bufferCreateInfo, NULL, buffer))
		return -1;

	VkMemoryRequirements bufferMemoryRequirements;
	vkGetBufferMemoryRequirements(engine.vulkanContext.device, *buffer, &bufferMemoryRequirements);

    // TODO check this ?
	// bufferMemoryRequirements.alignment;

	VkMemoryAllocateInfo memoryAllocateInfo = { VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
	memoryAllocateInfo.allocationSize = bufferMemoryRequirements.size;
	findMemoryTypeIndex(bufferMemoryRequirements.memoryTypeBits, properties, &memoryAllocateInfo.memoryTypeIndex);

	if(vkAllocateMemory(engine.vulkanContext.device, &memoryAllocateInfo, NULL, vertexBufferMemory))
		return -1;

	// only one vertex buffer that takes all the memory, so offset is 0
	if(vkBindBufferMemory(engine.vulkanContext.device, *buffer, *vertexBufferMemory, 0))
		return -1;

	return 0;
}


int copyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size)
{
	VkCommandBufferAllocateInfo allocInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO };
	allocInfo.commandPool = engine.vulkanContext.bufferCopyCommandPool;
	allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount = 1;

	VkCommandBuffer commandBuffer;
	VkResult result = vkAllocateCommandBuffers(engine.vulkanContext.device, &allocInfo, &commandBuffer);

	if (result != VK_SUCCESS)
	{
		fprintf(stderr, "Failed to allocate command buffer.\n");

		return result;
	}

	VkCommandBufferBeginInfo beginInfo = { VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO };
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	vkBeginCommandBuffer(commandBuffer, &beginInfo);

	VkBufferCopy copyRegion;
	copyRegion.srcOffset = 0;
	copyRegion.dstOffset = 0;
	copyRegion.size = size;

	vkCmdCopyBuffer(commandBuffer, src, dst, 1, &copyRegion);
	vkEndCommandBuffer(commandBuffer);

	VkSubmitInfo submitInfo = { VK_STRUCTURE_TYPE_SUBMIT_INFO };
	submitInfo.commandBufferCount = 1;
	submitInfo.pCommandBuffers = &commandBuffer;

	vkQueueSubmit(engine.vulkanContext.graphicQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(engine.vulkanContext.graphicQueue); // here we just wait, but could do multiple operations in // and use a fence to wait them all
	vkFreeCommandBuffers(engine.vulkanContext.device, engine.vulkanContext.bufferCopyCommandPool, 1, &commandBuffer);

	return VK_SUCCESS;
}


int findMemoryTypeIndex(uint32_t typeFilter, VkMemoryPropertyFlags properties, uint32_t* index)
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
