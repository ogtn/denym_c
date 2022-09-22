#ifndef _buffer_h_
#define _buffer_h_


#include "denym_common.h"


int createBuffer(VkDeviceSize size, VkBuffer *buffer, VkDeviceMemory *bufferMemory, VkMemoryPropertyFlags properties, VkBufferUsageFlags bufferUsage);

int createVertexBuffer(VkDeviceSize size, VkBuffer* buffer, VkDeviceMemory* bufferMemory, void* src);

int createBufferWithStaging(VkDeviceSize size, VkBuffer* buffer, VkDeviceMemory* bufferMemory, VkBufferUsageFlags bufferUsage, void* src);

int createVertexBufferWithStaging(VkDeviceSize size, VkBuffer* buffer, VkDeviceMemory* bufferMemory, void* src);

int createIndexBufferWithStaging(VkDeviceSize size, VkBuffer* buffer, VkDeviceMemory* bufferMemory, void* src);

int copyBuffer(VkBuffer src, VkBuffer dst, VkDeviceSize size);

int initiateCopyCommandBuffer(VkCommandBuffer *commandBuffer);

void terminateCopyCommandBuffer(VkCommandBuffer commandBuffer);

int findMemoryTypeIndex(uint32_t typeFilter, VkMemoryPropertyFlags properties, uint32_t* index);


#endif
