#pragma once

#include "gvkcommon.h"

struct GBuffer
{
	VkBuffer       Buffer;
	VkDeviceMemory Memory;
	void*          Data;
	VkDeviceSize   Size;
};

GBuffer CreateBuffer(VkDevice                                Device,
                     const VkPhysicalDeviceMemoryProperties& MemoryProperties,
                     VkDeviceSize                            Size,
                     VkBufferUsageFlags                      Usage);

void DestroyBuffer(VkDevice Device, GBuffer* Buffer);
