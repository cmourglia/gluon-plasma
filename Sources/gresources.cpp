#include "gresources.h"

#include "utils.h"

#include <loguru.hpp>

#include <cmath>

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

VkBufferMemoryBarrier BufferBarrier(VkBuffer Buffer, VkAccessFlags SrcAccessMask, VkAccessFlags DstAccessMask)
{
	VkBufferMemoryBarrier Barrier = {VK_STRUCTURE_TYPE_BUFFER_MEMORY_BARRIER};
	Barrier.srcAccessMask         = SrcAccessMask;
	Barrier.dstAccessMask         = DstAccessMask;
	Barrier.offset                = 0;
	Barrier.size                  = VK_WHOLE_SIZE;
	Barrier.srcQueueFamilyIndex   = VK_QUEUE_FAMILY_IGNORED;
	Barrier.dstQueueFamilyIndex   = VK_QUEUE_FAMILY_IGNORED;
	Barrier.buffer                = Buffer;

	return Barrier;
}

void DestroyBuffer(VkDevice Device, GBuffer* Buffer)
{
	vkFreeMemory(Device, Buffer->Memory, nullptr);
	vkDestroyBuffer(Device, Buffer->Buffer, nullptr);
}

VkImageMemoryBarrier ImageBarrier(VkImage       Image,
                                  VkAccessFlags SrcAccessMask,
                                  VkAccessFlags DstAccessMask,
                                  VkImageLayout OldLayout,
                                  VkImageLayout NewLayout)
{
	VkImageMemoryBarrier Barrier        = {VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER};
	Barrier.srcAccessMask               = SrcAccessMask;
	Barrier.dstAccessMask               = DstAccessMask;
	Barrier.oldLayout                   = OldLayout;
	Barrier.newLayout                   = NewLayout;
	Barrier.srcQueueFamilyIndex         = VK_QUEUE_FAMILY_IGNORED;
	Barrier.dstQueueFamilyIndex         = VK_QUEUE_FAMILY_IGNORED;
	Barrier.image                       = Image;
	Barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	Barrier.subresourceRange.levelCount = VK_REMAINING_MIP_LEVELS;
	Barrier.subresourceRange.layerCount = VK_REMAINING_ARRAY_LAYERS;

	return Barrier;
}

GImage CreateImage(VkDevice                                Device,
                   const VkPhysicalDeviceMemoryProperties& MemoryProperties,
                   u32                                     Width,
                   u32                                     Height,
                   u32                                     MipLevels,
                   VkFormat                                Format,
                   VkImageUsageFlags                       Usage)
{
	VkImageCreateInfo CreateInfo = {VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO};
	CreateInfo.imageType         = VK_IMAGE_TYPE_2D;
	CreateInfo.format            = Format;
	CreateInfo.extent            = {Width, Height, 1};
	CreateInfo.mipLevels         = MipLevels;
	CreateInfo.arrayLayers       = 1;
	CreateInfo.samples           = VK_SAMPLE_COUNT_1_BIT;
	CreateInfo.tiling            = VK_IMAGE_TILING_OPTIMAL;
	CreateInfo.usage             = Usage;
	CreateInfo.initialLayout     = VK_IMAGE_LAYOUT_UNDEFINED;

	VkImage Image;
	VK_CHECK(vkCreateImage(Device, &CreateInfo, nullptr, &Image));

	VkMemoryRequirements MemoryRequirements;
	vkGetImageMemoryRequirements(Device, Image, &MemoryRequirements);

	u32 MemoryTypeIndex = SelectMemoryType(MemoryProperties, MemoryRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	assert(MemoryTypeIndex != ~0u);

	VkMemoryAllocateInfo AllocInfo = {VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO};
	AllocInfo.allocationSize       = MemoryRequirements.size;
	AllocInfo.memoryTypeIndex      = MemoryTypeIndex;

	VkDeviceMemory Memory = 0;
	VK_CHECK(vkAllocateMemory(Device, &AllocInfo, 0, &Memory));

	VK_CHECK(vkBindImageMemory(Device, Image, Memory, 0));

	GImage Result    = {};
	Result.Image     = Image;
	Result.ImageView = CreateImageView(Device, Image, Format, 0, MipLevels);
	Result.Memory    = Memory;

	return Result;
}

VkImageView CreateImageView(VkDevice Device, VkImage Image, VkFormat ImageFormat, u32 MipLevel, u32 LevelCount)
{
	const VkImageAspectFlags AspectMask = (ImageFormat == VK_FORMAT_D32_SFLOAT) ? VK_IMAGE_ASPECT_DEPTH_BIT : VK_IMAGE_ASPECT_COLOR_BIT;

	VkImageViewCreateInfo CreateInfo         = {VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO};
	CreateInfo.image                         = Image;
	CreateInfo.viewType                      = VK_IMAGE_VIEW_TYPE_2D;
	CreateInfo.format                        = ImageFormat;
	CreateInfo.subresourceRange.aspectMask   = AspectMask;
	CreateInfo.subresourceRange.baseMipLevel = MipLevel;
	CreateInfo.subresourceRange.levelCount   = LevelCount;
	CreateInfo.subresourceRange.layerCount   = 1;

	VkImageView ImageView;
	VK_CHECK(vkCreateImageView(Device, &CreateInfo, nullptr, &ImageView));

	return ImageView;
}

VkSampler CreateSampler(VkDevice Device, VkSamplerReductionMode ReductionMode)
{
	VkSamplerCreateInfo CreateInfo = {VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO};
	CreateInfo.magFilter           = VK_FILTER_LINEAR;
	CreateInfo.minFilter           = VK_FILTER_LINEAR;
	CreateInfo.mipmapMode          = VK_SAMPLER_MIPMAP_MODE_NEAREST;
	CreateInfo.addressModeU        = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	CreateInfo.addressModeV        = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	CreateInfo.addressModeW        = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	CreateInfo.minLod              = 0;
	CreateInfo.maxLod              = 16.0f;

	if (ReductionMode != VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE)
	{
		VkSamplerReductionModeCreateInfo ReductionCreateInfo = {VK_STRUCTURE_TYPE_SAMPLER_REDUCTION_MODE_CREATE_INFO};
		ReductionCreateInfo.reductionMode                    = ReductionMode;
		CreateInfo.pNext                                     = &ReductionCreateInfo;
	}

	VkSampler Sampler;
	VK_CHECK(vkCreateSampler(Device, &CreateInfo, nullptr, &Sampler));

	return Sampler;
}

u32 GetImageMipLevels(u32 Width, u32 Height)
{
	const u32 Result = (u32)log2f((f32)Min(Width, Height));
	return Result;
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
