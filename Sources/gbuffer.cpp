#include "gbuffer.h"

#include <loguru.hpp>

static u32 SelectMemoryType(const VkPhysicalDeviceMemoryProperties& MemoryProperties, u32 MemoryTypeBits, u32 Flags);

GBuffer CreateBuffer(VkDevice Device, const VkPhysicalDeviceMemoryProperties& MemoryProperties, VkDeviceSize Size, VkBufferUsageFlags Usage)
{
	VkBufferCreateInfo CreateInfo = {VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO};
	CreateInfo.size               = Size;
	CreateInfo.usage              = Usage;

	GBuffer Buffer;
	VK_CHECK(vkCreateBuffer(Device, &CreateInfo, nullptr, &Buffer.Buffer));

	VkMemoryRequirements MemoryRequirements;
	vkGetBufferMemoryRequirements(Device, Buffer.Buffer, &MemoryRequirements);

	u32 MemoryTypeIndex = SelectMemoryType(MemoryProperties,
	                                       MemoryRequirements.memoryTypeBits,
	                                       VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	VkMemoryAllocateInfo AllocInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
	AllocInfo.allocationSize       = MemoryRequirements.size;
	AllocInfo.memoryTypeIndex      = MemoryTypeIndex;

	VK_CHECK(vkAllocateMemory(Device, &AllocInfo, nullptr, &Buffer.Memory));
	VK_CHECK(vkBindBufferMemory(Device, Buffer.Buffer, Buffer.Memory, 0));
	VK_CHECK(vkMapMemory(Device, Buffer.Memory, 0, Size, 0, &Buffer.Data));

	Buffer.Size = Size;

	return Buffer;
}

void DestroyBuffer(VkDevice Device, GBuffer* Buffer)
{
	vkFreeMemory(Device, Buffer->Memory, nullptr);
	vkDestroyBuffer(Device, Buffer->Buffer, nullptr);
}

static u32 SelectMemoryType(const VkPhysicalDeviceMemoryProperties& MemoryProperties, u32 MemoryTypeBits, u32 Flags)
{
	for (u32 Index = 0; Index < MemoryProperties.memoryTypeCount; ++Index)
	{
		if ((MemoryTypeBits & (1 << Index)) != 0 && (MemoryProperties.memoryTypes[Index].propertyFlags & Flags) == Flags)
		{
			return Index;
		}
	}

	LOG_F(ERROR, "No  compatible memory index");
	assert(false);

	return ~0u;
}
