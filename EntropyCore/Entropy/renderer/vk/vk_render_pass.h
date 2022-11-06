#pragma once
#include <vulkan/vulkan.h>
#include "../render_pass.h"

namespace et
{
	class VulkanRenderpass : public Renderpass
	{
	public:
		VulkanRenderpass(const RenderpassCreateInfo& createInfo);
		~VulkanRenderpass();
		
		operator VkRenderPass() const { return mRenderpass; }

		virtual void Bind(Ref<Framebuffer> framebuffer) override;
		virtual void BindNextSubpass() override;
		virtual void UnBind() override;

	private:
		void CreateRenderpass(
			const std::vector<VkAttachmentDescription>& attachmentDescriptions,
			const std::vector<VkSubpassDescription>& subpasses,
			const std::vector<VkSubpassDependency>& subpassDependencies);

		VkRenderPass mRenderpass = VK_NULL_HANDLE;
	};
}