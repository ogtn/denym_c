#ifndef _buffer_h_
#define _buffer_h_


#include "denym_common.h"


int bufferCreate(VkDeviceSize size, VkBuffer *buffer, VkDeviceMemory *bufferMemory, VkMemoryPropertyFlags properties, VkBufferUsageFlags bufferUsage);

int bufferCreateVertex(VkDeviceSize size, VkBuffer* buffer, VkDeviceMemory* bufferMemory, void* src);

int bufferCreateWithStaging(VkDeviceSize size, VkBuffer* buffer, VkDeviceMemory* bufferMemory, VkBufferUsageFlags bufferUsage, void* src);

int bufferCreateVertexWithStaging(VkDeviceSize size, VkBuffer* buffer, VkDeviceMemory* bufferMemory, void* src);

int bufferCreateIndexWithStaging(VkDeviceSize size, VkBuffer* buffer, VkDeviceMemory* bufferMemory, void* src);

int bufferCopy(VkBuffer src, VkBuffer dst, VkDeviceSize size);

int bufferInitiateCopyCmdBuffer(VkCommandBuffer *commandBuffer);

void bufferTerminateCopyCmdBuffer(VkCommandBuffer commandBuffer);

int bufferFindMemoryTypeIndex(uint32_t typeFilter, VkMemoryPropertyFlags properties, uint32_t* index);


#endif
