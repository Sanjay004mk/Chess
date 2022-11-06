#include <etpch.h>

#include "vkapi.h"

namespace et
{
	const std::vector<AttachmentDescription> attachmentDescriptions =
	{
		{
			TextureFormat::B8G8R8A8Unorm,		// format
			SampleCount::Num1,					// samples
			AttachmentLoadOp::Load,				// loadOp
			AttachmentStoreOp::Store,			// storeOp
			AttachmentLoadOp::DontCare,			// stencilLoadOp
			AttachmentStoreOp::DontCare,		// stencilStoreOp
			ImageLayout::ColorAttachment,		// initialLayout
			ImageLayout::ColorAttachment		// finalLayout
		},
		{
			TextureFormat::D24UnormS8Uint,		// format
			SampleCount::Num1,					// samples
			AttachmentLoadOp::Clear,			// loadOp
			AttachmentStoreOp::Store,			// storeOp
			AttachmentLoadOp::Clear,			// stencilLoadOp
			AttachmentStoreOp::Store,			// stencilStoreOp
			ImageLayout::Undefined,				// initialLayout
			ImageLayout::DepthStencilAttachment	// finalLayout
		},
		{
			TextureFormat::R32Sint,				// format
			SampleCount::Num1,					// samples
			AttachmentLoadOp::Clear,			// loadOp
			AttachmentStoreOp::Store,			// storeOp
			AttachmentLoadOp::DontCare,			// stencilLoadOp
			AttachmentStoreOp::DontCare,		// stencilStoreOp
			ImageLayout::Undefined,				// initialLayout
			ImageLayout::ShaderReadOnly			// finalLayout
		}
	};

	const std::vector<Attachment> colorAttachment =
	{
		{
			0,									// attachment
			ImageLayout::ColorAttachment
		},
		{
			2,									// attachment
			ImageLayout::ColorAttachment
		}
	};

	const Attachment depthStencilAttachment =
	{
		1,										// attachment
		ImageLayout::DepthStencilAttachment
	};

	const SubpassDescription subpassDescription =
	{
		colorAttachment,											// colorAttachments
		{ depthStencilAttachment },    								// depthStencilAttachment
		{},															// resolveAttachment
		0,															// srcSubpass
		(uint32_t)(~0),												// dstSubpass
		PipelineStage::ColorAttachmentOutput,						// srcStage
		PipelineStage::FragmentShader,								// dstStage
		AccessFlags::ColorAttachmentWrite,							// srcAccess
		AccessFlags::ShaderRead,									// dstAccess
		PipelineBindPoint::Graphics									// pipelineBindPoint
	};

	const RenderpassCreateInfo Renderpass::BasicRenderpassCreateInfo =
	{
		attachmentDescriptions,
		{ subpassDescription }
	};

	VulkanRenderpass::VulkanRenderpass(const RenderpassCreateInfo& createInfo)
	{
		std::vector<VkAttachmentDescription> attachmentDescriptions(createInfo.attachments.size());

		for (size_t i = 0; i < attachmentDescriptions.size(); i++)
		{
			attachmentDescriptions[i].flags = 0;
			attachmentDescriptions[i].finalLayout = (VkImageLayout)createInfo.attachments[i].finalLayout;
			attachmentDescriptions[i].initialLayout = (VkImageLayout)createInfo.attachments[i].initialLayout;
			attachmentDescriptions[i].format = (VkFormat)createInfo.attachments[i].format;
			attachmentDescriptions[i].samples = (VkSampleCountFlagBits)createInfo.attachments[i].samples;
			attachmentDescriptions[i].loadOp = (VkAttachmentLoadOp)createInfo.attachments[i].loadOp;
			attachmentDescriptions[i].storeOp = (VkAttachmentStoreOp)createInfo.attachments[i].storeOp;
			attachmentDescriptions[i].stencilLoadOp = (VkAttachmentLoadOp)createInfo.attachments[i].stencilLoadOp;
			attachmentDescriptions[i].stencilStoreOp = (VkAttachmentStoreOp)createInfo.attachments[i].stencilStoreOp;
		}

		std::vector<VkSubpassDescription> subpasses(createInfo.subpassDesc.size());
		std::vector<VkSubpassDependency> subpassDependencies(createInfo.subpassDesc.size());
		for (size_t i = 0; i < createInfo.subpassDesc.size(); i++)
		{
			auto& subpass = subpasses[i];
			auto& subpassDep = subpassDependencies[i];
			auto& subpassDesc = createInfo.subpassDesc[i];
			subpass.colorAttachmentCount = (uint32_t)subpassDesc.colorAttachments.size();
			subpass.flags = 0;
			subpass.inputAttachmentCount = 0;
			subpass.pInputAttachments = nullptr;
			subpass.pColorAttachments = (VkAttachmentReference*)subpassDesc.colorAttachments.data();
			if (subpassDesc.depthStencilAttachments.empty())
				subpass.pDepthStencilAttachment = nullptr;
			else
				subpass.pDepthStencilAttachment = (VkAttachmentReference*)subpassDesc.depthStencilAttachments.data();
			subpass.pipelineBindPoint = (VkPipelineBindPoint)subpassDesc.pipelineBindPoint;
			subpass.preserveAttachmentCount = 0;
			subpass.pPreserveAttachments = nullptr;
			if (subpassDesc.resolveAttachments.empty())
				subpass.pResolveAttachments = nullptr;
			else
				subpass.pResolveAttachments = (VkAttachmentReference*)subpassDesc.resolveAttachments.data();

			subpassDep.dependencyFlags = 0;
			subpassDep.srcSubpass = subpassDesc.srcSubpass;
			subpassDep.dstSubpass = subpassDesc.dstSubpass;
			subpassDep.srcAccessMask = (VkAccessFlags)subpassDesc.srcAccess;
			subpassDep.dstAccessMask = (VkAccessFlags)subpassDesc.dstAccess;
			subpassDep.srcStageMask = (VkPipelineStageFlags)subpassDesc.srcStage;
			subpassDep.dstStageMask = (VkPipelineStageFlags)subpassDesc.dstStage;
		}		

		CreateRenderpass(attachmentDescriptions, subpasses, subpassDependencies);
	}

	VulkanRenderpass::~VulkanRenderpass()
	{
		vkDestroyRenderPass(VulkanAPI::GetDevice(), mRenderpass, nullptr);
	}

	void VulkanRenderpass::CreateRenderpass(
		const std::vector<VkAttachmentDescription>& attachmentDescriptions,
		const std::vector<VkSubpassDescription>& subpasses,
		const std::vector<VkSubpassDependency>& subpassDependencies)
	{
		VkRenderPassCreateInfo renderPassInfo{};
		renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		renderPassInfo.attachmentCount = (uint32_t)(attachmentDescriptions.size());
		renderPassInfo.pAttachments = attachmentDescriptions.data();
		renderPassInfo.subpassCount = (uint32_t)subpasses.size();
		renderPassInfo.pSubpasses = subpasses.data();
		renderPassInfo.dependencyCount = (uint32_t)subpassDependencies.size();
		renderPassInfo.pDependencies = subpassDependencies.data();

		VkResult err = vkCreateRenderPass(VulkanAPI::GetDevice(), &renderPassInfo, nullptr, &mRenderpass);
		ET_VK_ASSERT_MSG(err, "Failed to create renderpass");
	}

	Ref<Renderpass> CreateRenderPass(const RenderpassCreateInfo& createInfo)
	{
		return CreateRef<VulkanRenderpass>(createInfo);
	}
}