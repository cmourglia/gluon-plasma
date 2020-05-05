#include "gshader.h"

#include <loguru.hpp>

#include <vulkan/spirv.h>

#include <stdlib.h>
#include <stdio.h>

#include <vector>

struct GIdentifier
{
	u32 Opcode;
	u32 TypeID;
	u32 StorageClass;
	u32 Binding;
	u32 Set;
};

static VkShaderStageFlagBits GetShaderStage(SpvExecutionModel ExecutionModel)
{
	switch (ExecutionModel)
	{
		case SpvExecutionModelVertex:
			return VK_SHADER_STAGE_VERTEX_BIT;
		case SpvExecutionModelFragment:
			return VK_SHADER_STAGE_FRAGMENT_BIT;
		case SpvExecutionModelGLCompute:
			return VK_SHADER_STAGE_COMPUTE_BIT;
		default:
			assert(!"Unsupported shader type");
	}

	return (VkShaderStageFlagBits)0;
}

static void ParseShader(GShader* Shader, const u32* Code, u64 CodeSize)
{
	assert(Code[0] == SpvMagicNumber);

	u32 IdentifierBound = Code[3];

	std::vector<GIdentifier> Identifiers(IdentifierBound);

	const u32* Ptr = Code + 5;

	while (Ptr != Code + CodeSize)
	{
		u16 Opcode    = (u16)Ptr[0];
		u16 WordCount = (u16)(Ptr[0] >> 16);

		switch (Opcode)
		{
			case SpvOpEntryPoint:
			{
				assert(WordCount >= 2);
				Shader->Stage = GetShaderStage((SpvExecutionModel)Ptr[1]);
			}
			break;

			case SpvOpExecutionMode:
			{
				assert(WordCount >= 3);
				u32 Mode = Ptr[2];

				switch (Mode)
				{
					case SpvExecutionModeLocalSize:
						assert(WordCount == 6);
						Shader->LocalSizeX = Ptr[3];
						Shader->LocalSizeY = Ptr[4];
						Shader->LocalSizeZ = Ptr[5];
				}
			}
			break;

			case SpvOpDecorate:
			{
				assert(WordCount >= 3);

				u32 Identifier = Ptr[1];
				assert(Identifier < IdentifierBound);

				switch (Ptr[2])
				{
					case SpvDecorationDescriptorSet:
						assert(WordCount == 4);
						Identifiers[Identifier].Set = Ptr[3];
						break;
					case SpvDecorationBinding:
						assert(WordCount == 4);
						Identifiers[Identifier].Binding = Ptr[3];
						break;
				}
			}
			break;

			case SpvOpTypeStruct:
			case SpvOpTypeImage:
			case SpvOpTypeSampler:
			case SpvOpTypeSampledImage:
			{
				assert(WordCount >= 2);

				u32 Identifier = Ptr[1];
				assert(Identifier < IdentifierBound);

				assert(Identifiers[Identifier].Opcode == 0);
				Identifiers[Identifier].Opcode = Opcode;
			}
			break;

			case SpvOpTypePointer:
			{
				assert(WordCount == 4);

				u32 Identifier = Ptr[1];
				assert(Identifier < IdentifierBound);

				assert(Identifiers[Identifier].Opcode == 0);
				Identifiers[Identifier].Opcode       = Opcode;
				Identifiers[Identifier].TypeID       = Ptr[3];
				Identifiers[Identifier].StorageClass = Ptr[2];
			}
			break;

			case SpvOpVariable:
			{
				assert(WordCount >= 4);

				u32 Identifier = Ptr[2];
				assert(Identifier < IdentifierBound);

				assert(Identifiers[Identifier].Opcode == 0);
				Identifiers[Identifier].Opcode       = Opcode;
				Identifiers[Identifier].TypeID       = Ptr[1];
				Identifiers[Identifier].StorageClass = Ptr[3];
			}
			break;
		}

		assert(Ptr + WordCount <= Code + CodeSize);
		Ptr += WordCount;
	}

	for (GIdentifier& Identifier : Identifiers)
	{
		if (Identifier.Opcode == SpvOpVariable &&
		    (Identifier.StorageClass == SpvStorageClassUniform || Identifier.StorageClass == SpvStorageClassUniformConstant ||
		     Identifier.StorageClass == SpvStorageClassStorageBuffer))
		{
			assert(Identifier.Set == 0);
			assert(Identifier.Binding < 32);
			assert(Identifiers[Identifier.TypeID].Opcode == SpvOpTypePointer);

			assert((Shader->ResourceMask & (1 << Identifier.Binding)) == 0);

			u32 TypeKind = Identifiers[Identifiers[Identifier.TypeID].TypeID].Opcode;

			switch (TypeKind)
			{
				case SpvOpTypeStruct:
					Shader->ResourceTypes[Identifier.Binding] = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
					Shader->ResourceMask |= 1 << Identifier.Binding;
					break;

				case SpvOpTypeImage:
					Shader->ResourceTypes[Identifier.Binding] = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
					Shader->ResourceMask |= 1 << Identifier.Binding;
					break;

				case SpvOpTypeSampler:
					Shader->ResourceTypes[Identifier.Binding] = VK_DESCRIPTOR_TYPE_SAMPLER;
					Shader->ResourceMask |= 1 << Identifier.Binding;
					break;

				case SpvOpTypeSampledImage:
					Shader->ResourceTypes[Identifier.Binding] = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
					Shader->ResourceMask |= 1 << Identifier.Binding;
					break;

				default:
					assert(!"Unknown resource type");
			}
		}

		if (Identifier.Opcode == SpvOpVariable && Identifier.StorageClass == SpvStorageClassPushConstant)
		{
			assert(!"Shader push constants are not handled yet");
			Shader->UsesPushConstants = true;
		}
	}
}

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
	assert(Read == Size && Size % 4 == 0);

	fclose(File);

	VkShaderModuleCreateInfo CreateInfo = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO};
	CreateInfo.codeSize                 = Size;
	CreateInfo.pCode                    = (u32*)Bytes;

	VK_CHECK(vkCreateShaderModule(Device, &CreateInfo, nullptr, &Shader.Module));

	ParseShader(&Shader, (u32*)Bytes, Size / 4);

	free(Bytes);

	return Shader;
}

void DestroyShader(VkDevice Device, GShader* Shader)
{
	vkDestroyShaderModule(Device, Shader->Module, nullptr);
}

static u32 GetShaderResources(GShaders Shaders, VkDescriptorType (&ResourceTypes)[32])
{
	u32 ResourceMask = 0;

	for (const GShader* Shader : Shaders)
	{
		for (u32 BindingIndex = 0; BindingIndex < 32; ++BindingIndex)
		{
			if (Shader->ResourceMask & (1 << BindingIndex))
			{
				if (ResourceMask & (1 << BindingIndex))
				{
					assert(ResourceTypes[BindingIndex] == Shader->ResourceTypes[BindingIndex]);
				}
				else
				{
					ResourceTypes[BindingIndex] = Shader->ResourceTypes[BindingIndex];
					ResourceMask |= 1 << BindingIndex;
				}
			}
		}
	}

	return ResourceMask;
}

static VkDescriptorSetLayout CreateSetLayout(VkDevice Device, GShaders Shaders)
{
	std::vector<VkDescriptorSetLayoutBinding> SetBindings;

	VkDescriptorType ResourceTypes[32] = {};
	u32              ResourceMask      = GetShaderResources(Shaders, ResourceTypes);

	for (u32 BindingIndex = 0; BindingIndex < 32; ++BindingIndex)
	{
		if (ResourceMask & (1 << BindingIndex))
		{
			VkDescriptorSetLayoutBinding Binding;
			Binding.binding         = BindingIndex;
			Binding.descriptorType  = ResourceTypes[BindingIndex];
			Binding.descriptorCount = 1;

			Binding.stageFlags = 0;

			for (const GShader* Shader : Shaders)
			{
				if (Shader->ResourceMask & (1 << BindingIndex))
				{
					Binding.stageFlags |= Shader->Stage;
				}
			}

			SetBindings.push_back(Binding);
		}
	}

	VkDescriptorSetLayoutCreateInfo CreateInfo = {VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO};
	CreateInfo.flags                           = VK_DESCRIPTOR_SET_LAYOUT_CREATE_PUSH_DESCRIPTOR_BIT_KHR;
	CreateInfo.bindingCount                    = (u32)SetBindings.size();
	CreateInfo.pBindings                       = SetBindings.data();

	VkDescriptorSetLayout SetLayout;
	VK_CHECK(vkCreateDescriptorSetLayout(Device, &CreateInfo, nullptr, &SetLayout));

	return SetLayout;
}

static VkPipelineLayout CreatePipelineLayout(VkDevice Device, VkDescriptorSetLayout SetLayout, GShaders Shaders)
{
	VkPipelineLayoutCreateInfo LayoutCreateInfo = {VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO};
	LayoutCreateInfo.setLayoutCount             = 1;
	LayoutCreateInfo.pSetLayouts                = &SetLayout;

	VkPipelineLayout Layout;
	VK_CHECK(vkCreatePipelineLayout(Device, &LayoutCreateInfo, nullptr, &Layout));

	return Layout;
}

static VkDescriptorUpdateTemplate CreateUpdateTemplate(VkDevice              Device,
                                                       VkPipelineBindPoint   BindPoint,
                                                       VkDescriptorSetLayout SetLayout,
                                                       VkPipelineLayout      Layout,
                                                       GShaders              Shaders)
{
	std::vector<VkDescriptorUpdateTemplateEntry> Entries;

	VkDescriptorType ResourceTypes[32] = {};
	u32              ResourceMask      = GetShaderResources(Shaders, ResourceTypes);

	for (u32 BindingIndex = 0; BindingIndex < 32; ++BindingIndex)
	{
		if (ResourceMask & (1 << BindingIndex))
		{
			VkDescriptorUpdateTemplateEntry Entry;
			Entry.dstBinding      = BindingIndex;
			Entry.dstArrayElement = 0;
			Entry.descriptorCount = 1;
			Entry.descriptorType  = ResourceTypes[BindingIndex];
			Entry.offset          = sizeof(GDescriptorInfo) * BindingIndex;
			Entry.stride          = sizeof(GDescriptorInfo);

			Entries.push_back(Entry);
		}
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

	return UpdateTemplate;
}

GProgram CreateProgram(VkDevice Device, VkPipelineBindPoint BindPoint, GShaders Shaders)
{
	GProgram Program = {};

	Program.BindPoint      = BindPoint;
	Program.SetLayout      = CreateSetLayout(Device, Shaders);
	Program.Layout         = CreatePipelineLayout(Device, Program.SetLayout, Shaders);
	Program.UpdateTemplate = CreateUpdateTemplate(Device, BindPoint, Program.SetLayout, Program.Layout, Shaders);

	return Program;
}

void DestroyProgram(VkDevice Device, GProgram* Program)
{
	vkDestroyDescriptorUpdateTemplate(Device, Program->UpdateTemplate, nullptr);
	vkDestroyPipelineLayout(Device, Program->Layout, nullptr);
	vkDestroyDescriptorSetLayout(Device, Program->SetLayout, nullptr);
}

VkPipeline CreateGraphicsPipeline(VkDevice Device, VkRenderPass RenderPass, VkPipelineLayout Layout, GShaders Shaders)
{
	VkGraphicsPipelineCreateInfo CreateInfo = {VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO};

	std::vector<VkPipelineShaderStageCreateInfo> Stages;

	for (const GShader* Shader : Shaders)
	{
		VkPipelineShaderStageCreateInfo Stage = {};
		Stage.sType                           = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		Stage.stage                           = Shader->Stage;
		Stage.module                          = Shader->Module;
		Stage.pName                           = "main";

		Stages.push_back(Stage);
	}

	CreateInfo.stageCount = (u32)Stages.size();
	CreateInfo.pStages    = Stages.data();

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
