#include <etpch.h>

#include "vkapi.h"

namespace et
{
	const PipelineCreateInfo Pipeline::BasicPipelineCreateInfo = PipelineCreateInfo();

	static VkFormat ToVkFormat(DataType format)
	{
		switch (format)
		{
		case DataType::Vec2:
			return VK_FORMAT_R32G32_SFLOAT;
		case DataType::Vec3:
			return VK_FORMAT_R32G32B32_SFLOAT;
		case DataType::Vec4:
			return VK_FORMAT_R32G32B32A32_SFLOAT;
		case DataType::Int32:
			return VK_FORMAT_R32_SINT;
		case DataType::Uint32:
			return VK_FORMAT_R32_UINT;
		default:
			ET_LOG_ERROR("unknown data format! {:#x}", (int32_t)format);
			return VK_FORMAT_R32G32B32_SFLOAT;
		}
	}

	static VkRect2D GetVkScissors(const Scissors& scissors)
	{
		if (scissors == Scissors())
			return { {0, 0}, VulkanAPI::GetSurfaceDetails().extent };

		VkOffset2D offs = { (int32_t)scissors.offset.x, (int32_t)scissors.offset.y };
		VkExtent2D extent = { (uint32_t)scissors.extent.x, (uint32_t)scissors.extent.y };

		return { offs, extent };
	}

	static VkViewport GetVkViewport(const Viewport& viewport)
	{
		if (viewport == Viewport())
			return { 0.f, 0.f, (float)VulkanAPI::GetSurfaceDetails().extent.width, (float)VulkanAPI::GetSurfaceDetails().extent.height, 0.f, 1.f };

		return *(VkViewport*)(&viewport);
	}

	VulkanPipeline::VulkanPipeline(const PipelineCreateInfo& createInfo)
	{
		Init(createInfo);
	}

	VulkanPipeline::~VulkanPipeline()
	{
		Destroy();
	}

	void VulkanPipeline::Init(const PipelineCreateInfo& createInfo)
	{
		VkGraphicsPipelineCreateInfo pipelineCreateInfo{};

		auto& shader = *((VulkanShader*)(createInfo.shader.get()));
		auto shaderStages = shader.GetShaderStages();

		std::vector<VkVertexInputAttributeDescription> vertexAttributeDesc(shader.vertexAttributes.size());
		for (size_t i = 0; i < vertexAttributeDesc.size(); i++)
		{
			auto& v = vertexAttributeDesc[i];
			auto& sv = shader.vertexAttributes[i];
			v.binding = sv.binding;
			v.format = ToVkFormat(sv.format);
			v.location = sv.location;
			v.offset = sv.offset;
		}
		std::vector<VkVertexInputBindingDescription> vertexBindingDesc(shader.vertexBindings.size());
		for (size_t i = 0; i < vertexBindingDesc.size(); i++)
		{
			auto& v = vertexBindingDesc[i];
			auto& sv = shader.vertexBindings[i];
			v.binding = sv.binding;
			v.inputRate = (VkVertexInputRate)sv.inputRate;
			v.stride = sv.stride;
		}

		VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
		vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertexInputInfo.vertexBindingDescriptionCount = (uint32_t)vertexBindingDesc.size();
		vertexInputInfo.pVertexBindingDescriptions = vertexBindingDesc.data();
		vertexInputInfo.vertexAttributeDescriptionCount = (uint32_t)vertexAttributeDesc.size();
		vertexInputInfo.pVertexAttributeDescriptions = vertexAttributeDesc.data();

		VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
		inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
		inputAssembly.primitiveRestartEnable = VK_FALSE;
		inputAssembly.topology = (VkPrimitiveTopology)createInfo.topology;

		VkPipelineMultisampleStateCreateInfo multisampling{};
		multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
		multisampling.sampleShadingEnable = VK_FALSE;
		multisampling.rasterizationSamples = (VkSampleCountFlagBits)createInfo.sampleCount;
		multisampling.minSampleShading = 1.0f;
		multisampling.pSampleMask = nullptr;
		multisampling.alphaToCoverageEnable = VK_FALSE;
		multisampling.alphaToOneEnable = VK_FALSE;

		VkPipelineColorBlendAttachmentState colorAttach{};
		colorAttach.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
		colorAttach.blendEnable = (VkBool32)createInfo.colorBlendEnable;
		colorAttach.colorBlendOp = VK_BLEND_OP_ADD;
		colorAttach.alphaBlendOp = VK_BLEND_OP_ADD;
		colorAttach.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
		colorAttach.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		colorAttach.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		colorAttach.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;

		std::vector<VkPipelineColorBlendAttachmentState> colorAttachs(createInfo.colorAttachmentCount, colorAttach);

		VkPipelineColorBlendStateCreateInfo colorBlend{};
		colorBlend.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
		colorBlend.logicOpEnable = VK_FALSE;
		colorBlend.logicOp = VK_LOGIC_OP_COPY;
		colorBlend.attachmentCount = (uint32_t)colorAttachs.size();
		colorBlend.pAttachments = colorAttachs.data();
		colorBlend.blendConstants[0] = 0.0f;
		colorBlend.blendConstants[1] = 0.0f;
		colorBlend.blendConstants[2] = 0.0f;
		colorBlend.blendConstants[3] = 0.0f;

		VkPipelineDepthStencilStateCreateInfo depthStencil{};
		depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
		depthStencil.depthWriteEnable = (VkBool32)createInfo.depthEnable;
		depthStencil.depthTestEnable = depthStencil.depthWriteEnable;
		depthStencil.depthBoundsTestEnable = VK_FALSE;
		depthStencil.depthCompareOp = (VkCompareOp)createInfo.depthCompareOp;
		depthStencil.maxDepthBounds = createInfo.depthMaxBound;
		depthStencil.minDepthBounds = createInfo.depthMinBound;
		depthStencil.stencilTestEnable = (VkBool32)createInfo.stencilEnable;
		depthStencil.back.compareOp = (VkCompareOp)createInfo.stencilCompareOp;
		depthStencil.back.depthFailOp = (VkStencilOp)createInfo.stencilDepthFailOp;
		depthStencil.back.failOp = (VkStencilOp)createInfo.stencilFailOp;
		depthStencil.back.passOp = (VkStencilOp)createInfo.stencilPassOp;
		depthStencil.back.compareMask = createInfo.stencilCompareMask;
		depthStencil.back.writeMask = createInfo.stencilWriteMask;
		depthStencil.back.reference = createInfo.stencilReference;
		depthStencil.front = depthStencil.back;

		VkPipelineRasterizationStateCreateInfo rasterizer{};
		rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
		rasterizer.cullMode = (VkCullModeFlags)createInfo.cullMode;
		rasterizer.depthBiasEnable = VK_FALSE;
		rasterizer.depthBiasClamp = rasterizer.depthBiasConstantFactor = rasterizer.depthBiasSlopeFactor = 0.f;
		rasterizer.depthClampEnable = VK_FALSE;
		rasterizer.frontFace = (VkFrontFace)createInfo.frontFace;
		rasterizer.rasterizerDiscardEnable = VK_FALSE;
		rasterizer.lineWidth = createInfo.lineWidth;
		rasterizer.polygonMode = (VkPolygonMode)createInfo.polygonMode;
		
		pipelineCreateInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		pipelineCreateInfo.layout = shader.GetPipelineLayout();
		pipelineCreateInfo.pColorBlendState = &colorBlend;
		pipelineCreateInfo.pDepthStencilState = &depthStencil;
		pipelineCreateInfo.pInputAssemblyState = &inputAssembly;
		pipelineCreateInfo.pMultisampleState = &multisampling;
		pipelineCreateInfo.pRasterizationState = &rasterizer;
		pipelineCreateInfo.stageCount = (uint32_t)shaderStages.size();
		pipelineCreateInfo.pStages = shaderStages.data();
		pipelineCreateInfo.pVertexInputState = &vertexInputInfo;
		pipelineCreateInfo.renderPass = (VkRenderPass)(*((VulkanRenderpass*)createInfo.renderpass.get()));
		pipelineCreateInfo.subpass = createInfo.subpass;
		pipelineCreateInfo.basePipelineHandle = VK_NULL_HANDLE;

		VkPipelineDynamicStateCreateInfo dynamicState{};
		VkDynamicState states[] = { VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR };
		dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
		dynamicState.dynamicStateCount = 2;
		dynamicState.pDynamicStates = states;

		VkRect2D scissors = GetVkScissors(createInfo.scissors);
		VkViewport viewport = GetVkViewport(createInfo.viewport);

		VkPipelineViewportStateCreateInfo viewportState{};
		viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
		viewportState.scissorCount = 1;
		viewportState.viewportCount = 1;
		viewportState.pScissors = &scissors;
		viewportState.pViewports = &viewport;

		if (createInfo.dynamicViewport)
		{
			pipelineCreateInfo.pDynamicState = &dynamicState;
		}
		else
			pipelineCreateInfo.pViewportState = &viewportState;

		VkResult err = vkCreateGraphicsPipelines(VulkanAPI::GetDevice(), VK_NULL_HANDLE, 1, &pipelineCreateInfo, nullptr, &mPipeline);
		ET_VK_ASSERT_MSG(err, "Failed to create graphics pipeline");
	}

	void VulkanPipeline::Destroy()
	{
		VulkanAPI::DeviceWaitIdle();
		vkDestroyPipeline(VulkanAPI::GetDevice(), mPipeline, nullptr);
	}

	Ref<Pipeline> CreatePipeline(const PipelineCreateInfo& createInfo)
	{
		return CreateRef<VulkanPipeline>(createInfo);
	}
}