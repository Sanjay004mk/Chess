#include <etpch.h>

#include "vk/vkapi.h"
#include "render_commands.h"

namespace et
{

	VkSemaphore RenderCommand_signalSemaphore = nullptr;
	static Ref<CommandPoolArray> commandPools;
	static Ref<CommandBuffer> currentCommandBuffer;
	static std::vector<std::vector<Ref<CommandBuffer>>> commandBuffers;
	static std::vector<std::vector<VkSemaphore>> syncSemaphores;
	static uint32_t cbIndex = 0;
	static std::unordered_map<uint32_t, std::tuple<std::optional<glm::vec4>, std::optional<float>, std::optional<uint32_t>>> clearColors;

	void RenderCommand::Init()
	{
		syncSemaphores.resize(maxCommandBuffers + 1);
		commandBuffers.resize(maxCommandBuffers);

		VkSemaphoreCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		syncSemaphores[0] = VulkanAPI::sImageAcquiredSempahores;
		for (size_t i = 1; i < syncSemaphores.size(); i++)
		{
			syncSemaphores[i].resize(VulkanAPI::sSurfaceDetails.imageCount);
			for (auto& s : syncSemaphores[i])
			{
				VkResult err = vkCreateSemaphore(VulkanAPI::GetDevice(), &createInfo, nullptr, &s);
				ET_VK_ASSERT_MSG(err, "Failed to create semaphore");
			}
		}

		commandPools = CreateRef<CommandPoolArray>(VulkanAPI::GetSurfaceDetails().imageCount);
		for (auto& cbs : commandBuffers)
		{
			cbs.resize(VulkanAPI::GetSurfaceDetails().imageCount);
			for (size_t i = 0; i < cbs.size(); i++)
				cbs[i] = CreateRef<CommandBuffer>((*commandPools)[(int32_t)i]);
		}
	}

	void RenderCommand::Shutdown()
	{
		currentCommandBuffer.reset();

		for(size_t i = 1; i < syncSemaphores.size(); i++)
			for (auto& s : syncSemaphores[i])
				vkDestroySemaphore(VulkanAPI::GetDevice(), s, nullptr);

		for (auto& cbs : commandBuffers)
			for (auto& c : cbs)
				c.reset();

		commandPools.reset();
	}

	void RenderCommand::Begin()
	{
		cbIndex = 0;
		VkResult err = vkResetCommandPool(VulkanAPI::GetDevice(), (*commandPools)[VulkanAPI::sCurrentFrame], 0);
		ET_VK_ASSERT_NO_MSG(err);
	}

	void RenderCommand::End()
	{
		RenderCommand_signalSemaphore = syncSemaphores[cbIndex][VulkanAPI::sCurrentFrame];
	}

	CommandBuffer* RenderCommand::GetCommandBuffer()
	{
		return currentCommandBuffer.get();
	}

	void RenderCommand::StartCommandBuffer()
	{
		if (cbIndex >= maxCommandBuffers)
		{
			ET_ASSERT_MSG_BREAK(false, "Maximum number of command buffers recorded!");
		}
		currentCommandBuffer = commandBuffers[cbIndex][VulkanAPI::sCurrentFrame];
		currentCommandBuffer->Begin();
	}

	void RenderCommand::EndCommandBuffer()
	{
		currentCommandBuffer->End();
		VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		currentCommandBuffer->Submit(&syncSemaphores[cbIndex][VulkanAPI::sCurrentFrame], 1, &waitStage, &syncSemaphores[cbIndex + 1][VulkanAPI::sCurrentFrame], 1);
		cbIndex++;
	}

	void RenderCommand::SetClearColor(const glm::vec4& __clearColor, uint32_t attachment)
	{
		std::get<std::optional<glm::vec4>>(clearColors[attachment]) = __clearColor;
	}

	void RenderCommand::SetClearColor(const glm::ivec4& r32, uint32_t attachment)
	{
		glm::vec4 c{};
		memcpy(&c, &r32, sizeof(glm::vec4));
		std::get<std::optional<glm::vec4>>(clearColors[attachment]) = c;
	}

	void RenderCommand::SetDepthClearColor(float __depthClearColor, uint32_t attachment)
	{
		std::get<std::optional<float>>(clearColors[attachment]) = __depthClearColor;
	}

	void RenderCommand::SetStencilClearColor(uint32_t __stencilClearColor, uint32_t attachment)
	{
		std::get<std::optional<uint32_t>>(clearColors[attachment]) = __stencilClearColor;
	}

	void RenderCommand::ResetClearColor()
	{
		clearColors.clear();
	}

	void RenderCommand::DrawIndexed(const VertexBuffer& vertexBuffer, const IndexBuffer& indexBuffer)
	{
		VkBuffer vertex = *((VulkanBuffer*)(vertexBuffer.buffer.get()));
		VkBuffer index = *((VulkanBuffer*)(indexBuffer.buffer.get()));
		VkDeviceSize offs[] = { 0 };
		VkCommandBuffer c = *currentCommandBuffer;

		vkCmdBindVertexBuffers(c, 0, 1, &vertex, offs);
		vkCmdBindIndexBuffer(c, index, 0, VK_INDEX_TYPE_UINT32);
		vkCmdDrawIndexed(c, (uint32_t)indexBuffer.count, 1, 0, 0, 0);
	}

	void RenderCommand::Draw(const VertexBuffer& vertexBuffer)
	{
		VkBuffer vertex = *((VulkanBuffer*)(vertexBuffer.buffer.get()));
		VkDeviceSize offs[] = { 0 };
		VkCommandBuffer c = *currentCommandBuffer;

		vkCmdBindVertexBuffers(c, 0, 1, &vertex, offs);
		vkCmdDraw(c, (uint32_t)vertexBuffer.count, 1, 0, 0);
	}

	void RenderCommand::Draw(uint32_t count)
	{
		VkCommandBuffer c = *currentCommandBuffer;

		vkCmdDraw(c, count, 1, 0, 0);
	}

	void VulkanPipeline::Bind(PipelineBindPoint bindPoint)
	{
		VkCommandBuffer c = *currentCommandBuffer;
		vkCmdBindPipeline(c, (VkPipelineBindPoint)bindPoint, mPipeline);
	}

	void VulkanPipeline::UnBind()
	{

	}

	void VulkanPipeline::SetDynamicViewport(const Viewport& viewport)
	{
		VkCommandBuffer c = *currentCommandBuffer;
		VkRect2D scissor{};
		scissor.offset = { 0, 0 };
		scissor.extent = { (uint32_t)viewport.width, (uint32_t)viewport.height };
		vkCmdSetViewport(c, 0, 1, (VkViewport*)&viewport);
		vkCmdSetScissor(c, 0, 1, &scissor);
	}

	void VulkanRenderpass::Bind(Ref<Framebuffer> framebuffer)
	{
		VkCommandBuffer c = *currentCommandBuffer;
		VkFramebuffer fb = *((VulkanFramebuffer*)(framebuffer.get()));
		VkRenderPassBeginInfo beginInfo{};
		beginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		std::vector<VkClearValue> clearValues(clearColors.size());
		for (auto& [attachment, values] : clearColors)
		{
			VkClearValue clearValue{};
			auto& [color, depth, stencil] = values;
			if (color.has_value())
				clearValue.color = { { color.value().x, color.value().y, color.value().z, color.value().w } };
			if (depth.has_value())
				clearValue.depthStencil.depth = depth.value();
			if (stencil.has_value())
				clearValue.depthStencil.stencil = stencil.value();

			clearValues[attachment] = clearValue;
		}

		beginInfo.clearValueCount = (uint32_t)clearValues.size();
		beginInfo.pClearValues = clearValues.data();
		beginInfo.framebuffer = fb;
		beginInfo.renderPass = mRenderpass;
		beginInfo.renderArea.extent = { framebuffer->width, framebuffer->height };

		vkCmdBeginRenderPass(c, &beginInfo, VK_SUBPASS_CONTENTS_INLINE);
	}

	void VulkanRenderpass::UnBind()
	{
		VkCommandBuffer c = *currentCommandBuffer;
		RenderCommand::ResetClearColor();
		vkCmdEndRenderPass(c);
	}

	void VulkanRenderpass::BindNextSubpass()
	{
		VkCommandBuffer c = *currentCommandBuffer;
		vkCmdNextSubpass(c, VK_SUBPASS_CONTENTS_INLINE);
	}

	void VulkanShader::Bind(PipelineBindPoint bindPoint, uint32_t dynamicOffsetCount, uint32_t* pDynamicOffsets)
	{
		if (mPool == VK_NULL_HANDLE)
			return;
		VkCommandBuffer c = *currentCommandBuffer;
		vkCmdBindDescriptorSets(c, (VkPipelineBindPoint)bindPoint, mPipelineLayout, 0, 1, &mSets[VulkanAPI::GetCurrentFrame()], dynamicOffsetCount, pDynamicOffsets);
	}

	void VulkanShader::UnBind()
	{

	}

	void VulkanShader::UploadPushConstant()
	{
		VkCommandBuffer c = *currentCommandBuffer;
		vkCmdPushConstants(c, mPipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, (uint32_t)pushConstant.size(), pushConstant.GetData());
	}
}