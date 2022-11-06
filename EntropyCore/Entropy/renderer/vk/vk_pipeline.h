#pragma once
#include <vulkan/vulkan.h>

#include "../pipeline.h"

namespace et
{
	class VulkanPipeline : public Pipeline
	{
	public:
		VulkanPipeline(const PipelineCreateInfo& createInfo);
		~VulkanPipeline();

		virtual void SetDynamicViewport(const Viewport& viewport) override;

		virtual void Bind(PipelineBindPoint bindPoint) override;
		virtual void UnBind() override;

		operator VkPipeline() const { return mPipeline; }

	private:
		void Init(const PipelineCreateInfo& createInfo);
		void Destroy();

		VkPipeline mPipeline = VK_NULL_HANDLE;
	};
}