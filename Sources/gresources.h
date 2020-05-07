#pragma once

#include "gvkcommon.h"

struct GBuffer
{
	VkBuffer       Buffer;
	VkDeviceMemory Memory;
	void*          Data;
	VkDeviceSize   Size;
};

struct GImage
{
	VkImage        Image;
	VkImageView    ImageView;
	VkDeviceMemory Memory;
};

GBuffer CreateBuffer(VkDevice                                Device,
                     const VkPhysicalDeviceMemoryProperties& MemoryProperties,
                     VkDeviceSize                            Size,
                     VkBufferUsageFlags                      Usage);

VkBufferMemoryBarrier BufferBarrier(VkBuffer Buffer, VkAccessFlags SrcAccessMask, VkAccessFlags DstAccessMask);

void DestroyBuffer(VkDevice Device, GBuffer* Buffer);

GImage CreateImage(VkDevice                                Device,
                   const VkPhysicalDeviceMemoryProperties& MemoryProperties,
                   u32                                     Width,
                   u32                                     Height,
                   u32                                     MipLevels,
                   VkFormat                                Format,
                   VkImageUsageFlags                       Usage);

VkImageMemoryBarrier ImageBarrier(VkImage       Image,
                                  VkAccessFlags SrcAccessMask,
                                  VkAccessFlags DstAccessMask,
                                  VkImageLayout OldLayout,
                                  VkImageLayout DstLayout);

VkImageView CreateImageView(VkDevice Device, VkImage Image, VkFormat Format, u32 MipLevel = 0, u32 LevelCount = 1);

VkSampler CreateSampler(VkDevice Device, VkSamplerReductionMode ReductionMode = VK_SAMPLER_REDUCTION_MODE_WEIGHTED_AVERAGE);

u32 GetImageMipLevels(u32 Width, u32 Height);

void DestroyImage(VkDevice Device, GImage* Image);
