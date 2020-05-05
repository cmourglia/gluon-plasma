#pragma once

#include "gvulkan.h"

struct GShader
{
	VkShaderModule Module;
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

GProgram CreateProgram(VkDevice Device, VkPipelineBindPoint BindPoint, const GShader& VertexShader, const GShader& FragmentShader);
void     DestroyProgram(VkDevice Device, GProgram* Program);

VkPipeline CreateGraphicsPipeline(VkDevice         Device,
                                  VkRenderPass     RenderPass,
                                  VkPipelineLayout Layout,
                                  const GShader&   VertexShader,
                                  const GShader&   FragmentShader);

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
