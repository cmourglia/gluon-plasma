#pragma once

#include "gvulkan.h"

#include <initializer_list>

struct GShader
{
	VkShaderModule        Module;
	VkShaderStageFlagBits Stage;

	VkDescriptorType ResourceTypes[32];
	u32              ResourceMask;

	u32 LocalSizeX;
	u32 LocalSizeY;
	u32 LocalSizeZ;

	bool UsesPushConstants;
};

GShader LoadShader(VkDevice Device, const char* Filename);
void    DestroyShader(VkDevice Device, GShader* Shader);

struct GProgram
{
	VkPipelineBindPoint        BindPoint;
	VkDescriptorSetLayout      SetLayout;
	VkPipelineLayout           Layout;
	VkDescriptorUpdateTemplate UpdateTemplate;
};

using GShaders = std::initializer_list<const GShader*>;

GProgram CreateProgram(VkDevice Device, VkPipelineBindPoint BindPoint, GShaders Shaders);
void     DestroyProgram(VkDevice Device, GProgram* Program);

VkPipeline CreateGraphicsPipeline(VkDevice Device, VkRenderPass RenderPass, VkPipelineLayout Layout, GShaders Shaders);

union GDescriptorInfo {
	VkDescriptorImageInfo  Image;
	VkDescriptorBufferInfo Buffer;

	GDescriptorInfo(VkSampler Sampler, VkImageView ImageView, VkImageLayout ImageLayout)
	{
		Image.sampler     = Sampler;
		Image.imageView   = ImageView;
		Image.imageLayout = ImageLayout;
	}

	GDescriptorInfo(VkImageView ImageView, VkImageLayout ImageLayout)
	{
		Image.sampler     = VK_NULL_HANDLE;
		Image.imageView   = ImageView;
		Image.imageLayout = ImageLayout;
	}

	GDescriptorInfo(VkBuffer pBuffer, VkDeviceSize Offset, VkDeviceSize Range)
	{
		Buffer.buffer = pBuffer;
		Buffer.offset = Offset;
		Buffer.range  = Range;
	}

	GDescriptorInfo(VkBuffer pBuffer)
	{
		Buffer.buffer = pBuffer;
		Buffer.offset = 0;
		Buffer.range  = VK_WHOLE_SIZE;
	}
};
