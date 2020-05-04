#include "gshader.h"

#include <stdlib.h>
#include <stdio.h>

#include <vector>

GShader LoadShader(VkDevice Device, const char* Filename)
{
	FILE* File = fopen(Filename, "rb");

	GShader Shader = {};

	if (nullptr == File)
	{
		return Shader;
	}

	fseek(File, 0, SEEK_END);
	const u64 Size = ftell(File);
	fseek(File, 0, SEEK_SET);

	u8*       Bytes = (u8*)malloc(Size);
	const u64 Read  = fread(Bytes, 1, Size, File);
	assert(Read == Size);

	fclose(File);

	VkShaderModuleCreateInfo CreateInfo = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
	CreateInfo.codeSize                 = Size;
	CreateInfo.pCode                    = (u32*)Bytes;

	VK_CHECK(vkCreateShaderModule(Device, &CreateInfo, nullptr, &Shader.Module));

	free(Bytes);

	return Shader;
}

void DestroyShader(VkDevice Device, GShader* Shader)
{
	vkDestroyShaderModule(Device, Shader->Module, nullptr);
}

static VkDescriptorSetLayout CreateSetLayout(VkDevice Device)
{
	std::vector<VkDescriptorSetLayoutBinding> SetBindings;

	{
		VkDescriptorSetLayoutBinding SetBinding;
		SetBinding.binding         = 0;
		SetBinding.descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		SetBinding.descriptorCount = 1;
		SetBinding.stageFlags      = VK_SHADER_STAGE_VERTEX_BIT;

		SetBindings.push_back(SetBinding);
	}

	VkDescriptorSetLayoutCreateInfo CreateInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
	CreateInfo.flags                           = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
	CreateInfo.bindingCount                    = (u32)SetBindings.size();
	CreateInfo.pBindings                       = SetBindings.data();

	VkDescriptorSetLayout SetLayout;
	VK_CHECK(vkCreateDescriptorSetLayout(Device, &CreateInfo, nullptr, &SetLayout));

	return SetLayout;
}

VkPipelineLayout CreatePipelineLayout(VkDevice Device, const GShader& VertexShader, const GShader& FragmentShader)
{
	VkDescriptorSetLayout SetLayout = CreateSetLayout(Device);

	VkPipelineLayoutCreateInfo LayoutCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
	LayoutCreateInfo.setLayoutCount             = 1;
	LayoutCreateInfo.pSetLayouts                = &SetLayout;

	VkPipelineLayout Layout;
	VK_CHECK(vkCreatePipelineLayout(Device, &LayoutCreateInfo, nullptr, &Layout));

	// TODO: We need to destroy this at some point
	// vkDestroyDescriptorSetLayout(Device, SetLayout, nullptr);

	return Layout;
}

VkDescriptorUpdateTemplate CreateUpdateTemplate(VkDevice Device, VkPipelineBindPoint BindPoint, VkPipelineLayout Layout)
{
	VkDescriptorSetLayout SetLayout = CreateSetLayout(Device);

	std::vector<VkDescriptorUpdateTemplateEntry> Entries;

	{
		VkDescriptorUpdateTemplateEntry Entry;
		Entry.dstBinding      = 0;
		Entry.dstArrayElement = 0;
		Entry.descriptorCount = 1;
		Entry.descriptorType  = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
		Entry.offset          = sizeof(GDescriptorInfo) * 0;
		Entry.stride          = sizeof(GDescriptorInfo);

		Entries.push_back(Entry);
	}

	VkDescriptorUpdateTemplateCreateInfo CreateInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_CREATE_INFO};
	CreateInfo.descriptorUpdateEntryCount           = (u32)Entries.size();
	CreateInfo.pDescriptorUpdateEntries             = Entries.data();
	CreateInfo.templateType                         = VK_DESCRIPTOR_UPDATE_TEMPLATE_TYPE_PUSH_DESCRIPTORS_KHR;
	CreateInfo.descriptorSetLayout                  = SetLayout;
	CreateInfo.pipelineBindPoint                    = BindPoint;
	CreateInfo.pipelineLayout                       = Layout;

	VkDescriptorUpdateTemplate UpdateTemplate;
	VK_CHECK(vkCreateDescriptorUpdateTemplate(Device, &CreateInfo, nullptr, &UpdateTemplate));

	// TODO: We need to destroy this at some point
	// vkDestroyDescriptorSetLayout(Device, SetLayout, nullptr);

	return UpdateTemplate;
}

VkPipeline CreateGraphicsPipeline(VkDevice         Device,
                                  VkRenderPass     RenderPass,
                                  VkPipelineLayout Layout,
                                  const GShader&   VertexShader,
                                  const GShader&   FragmentShader)
{
	VkGraphicsPipelineCreateInfo CreateInfo = {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};

	VkPipelineShaderStageCreateInfo Stages[2] = {};

	Stages[0].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	Stages[0].stage  = VK_SHADER_STAGE_VERTEX_BIT;
	Stages[0].module = VertexShader.Module;
	Stages[0].pName  = "main";

	Stages[1].sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	Stages[1].stage  = VK_SHADER_STAGE_FRAGMENT_BIT;
	Stages[1].module = FragmentShader.Module;
	Stages[1].pName  = "main";

	CreateInfo.stageCount = ARRAY_SIZE(Stages);
	CreateInfo.pStages    = Stages;

	VkPipelineVertexInputStateCreateInfo VertexInputState = {VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO};
	CreateInfo.pVertexInputState                          = &VertexInputState;

	VkPipelineInputAssemblyStateCreateInfo InputAssemblyState = {VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO};
	InputAssemblyState.topology                               = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	CreateInfo.pInputAssemblyState                            = &InputAssemblyState;

	VkPipelineViewportStateCreateInfo ViewportState = {VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO};
	ViewportState.viewportCount                     = 1;
	ViewportState.scissorCount                      = 1;
	CreateInfo.pViewportState                       = &ViewportState;

	// VkViewport Viewport = {};
	// Viewport.x          = 0.0f;
	// Viewport.y          = (f32)Swapchain.Extent.height;
	// Viewport.width      = (f32)Swapchain.Extent.width;
	// Viewport.height     = -(f32)Swapchain.Extent.height;
	// Viewport.minDepth   = 0.0f;
	// Viewport.maxDepth   = 1.0f;

	// VkRect2D Scissor = {};
	// Scissor.offset   = {0, 0};
	// Scissor.extent   = Swapchain.Extent;

	VkPipelineRasterizationStateCreateInfo RasterizationState = {VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO};
	RasterizationState.depthClampEnable                       = VK_FALSE;
	RasterizationState.rasterizerDiscardEnable                = VK_FALSE;
	RasterizationState.polygonMode                            = VK_POLYGON_MODE_FILL;
	RasterizationState.lineWidth                              = 1.0f;
	RasterizationState.depthBiasEnable                        = VK_FALSE;
	CreateInfo.pRasterizationState                            = &RasterizationState;

	VkPipelineMultisampleStateCreateInfo MultisampleState = {VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO};
	MultisampleState.sampleShadingEnable                  = VK_FALSE;
	MultisampleState.rasterizationSamples                 = VK_SAMPLE_COUNT_1_BIT;
	CreateInfo.pMultisampleState                          = &MultisampleState;

	VkPipelineDepthStencilStateCreateInfo DepthStencilState = {VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO};
	CreateInfo.pDepthStencilState                           = &DepthStencilState;

	VkPipelineColorBlendAttachmentState ColorAttachmentState = {};
	ColorAttachmentState.colorWriteMask                      = COLOR_MASK_RGBA;
	ColorAttachmentState.blendEnable                         = VK_FALSE;

	VkPipelineColorBlendStateCreateInfo ColorBlendState = {VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO};
	ColorBlendState.logicOpEnable                       = VK_FALSE;
	ColorBlendState.attachmentCount                     = 1;
	ColorBlendState.pAttachments                        = &ColorAttachmentState;
	CreateInfo.pColorBlendState                         = &ColorBlendState;

	VkDynamicState DynamicStates[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

	VkPipelineDynamicStateCreateInfo DynamicState = {VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO};
	DynamicState.dynamicStateCount                = ARRAY_SIZE(DynamicStates);
	DynamicState.pDynamicStates                   = DynamicStates;
	CreateInfo.pDynamicState                      = &DynamicState;

	CreateInfo.layout     = Layout;
	CreateInfo.renderPass = RenderPass;

	VkPipeline Pipeline;
	VK_CHECK(vkCreateGraphicsPipelines(Device, VK_NULL_HANDLE, 1, &CreateInfo, nullptr, &Pipeline));

	return Pipeline;
}
