#pragma once
#include <optional>

#include "Entropy/EntropyUtils.h"
#include "render_info.h"

namespace et
{
	class Framebuffer;

	struct Attachment
	{
		uint32_t attachment = 0;
		ImageLayout layout = ImageLayout::ColorAttachment;
	};

	struct AttachmentDescription
	{
		TextureFormat format;
		SampleCount samples = SampleCount::Num1;
		AttachmentLoadOp loadOp = AttachmentLoadOp::Load;
		AttachmentStoreOp storeOp = AttachmentStoreOp::Store;
		AttachmentLoadOp stencilLoadOp = AttachmentLoadOp::DontCare;
		AttachmentStoreOp stencilStoreOp = AttachmentStoreOp::DontCare;
		ImageLayout initialLayout = ImageLayout::Undefined;
		ImageLayout finalLayout = ImageLayout::ColorAttachment;
	};

	struct SubpassDescription
	{
		std::vector<Attachment> colorAttachments;
		std::vector<Attachment> depthStencilAttachments;
		std::vector<Attachment> resolveAttachments;
		uint32_t srcSubpass = 0;
		uint32_t dstSubpass = (~0);
		PipelineStage srcStage = PipelineStage::ColorAttachmentOutput;
		PipelineStage dstStage = PipelineStage::FragmentShader;
		AccessFlags srcAccess = AccessFlags::ColorAttachmentWrite;
		AccessFlags dstAccess = AccessFlags::ShaderRead;
		PipelineBindPoint pipelineBindPoint = PipelineBindPoint::Graphics;
	};

	struct RenderpassCreateInfo
	{
		std::vector<AttachmentDescription> attachments;
		std::vector<SubpassDescription> subpassDesc;
	};

	class Renderpass
	{
	public:
		Renderpass() = default;
		virtual ~Renderpass() {}

		virtual void Bind(Ref<Framebuffer> framebuffer) = 0;
		virtual void BindNextSubpass() = 0;
		virtual void UnBind() {}
	
		static const RenderpassCreateInfo BasicRenderpassCreateInfo;
	};


	Ref<Renderpass> CreateRenderPass(const RenderpassCreateInfo& createInfo);
}