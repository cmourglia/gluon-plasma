#pragma once

#include "gvulkan.h"

struct GShader
{
	VkShaderModule Module;
};

GShader LoadShader(VkDevice Device, const char* Filename);

VkPipelineLayout CreatePipelineLayout(VkDevice Device, const GShader& VertexShader, const GShader& FragmentShader);

VkPipeline CreateGraphicsPipeline(VkDevice         Device,
                                  VkRenderPass     RenderPass,
                                  VkPipelineLayout Layout,
                                  const GShader&   VertexShader,
                                  const GShader&   FragmentShader);

void DestroyShader(VkDevice Device, GShader* Shader);
