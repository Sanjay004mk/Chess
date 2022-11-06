#pragma once
#include <functional>
#include <vulkan/vulkan.h>

#include "../buffer.h"

namespace et
{
	void InitCommandPool();
	void DestroyCommandPool();

	class CommandBuffer
	{
	public:
		CommandBuffer(VkCommandPool commandPool = VK_NULL_HANDLE);
		CommandBuffer(const CommandBuffer&) = delete;
		~CommandBuffer();

		void Begin(VkCommandBufferUsageFlags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
		void End();
		void Submit();
		void Submit(VkSemaphore* pWaitSemaphores, uint32_t waitSemaphoresCount, VkPipelineStageFlags* pWaitDstStageFlags, VkSemaphore* pSignalSemaphores, uint32_t signalSemaphoreCount, VkFence fence = nullptr);

		operator VkCommandBuffer() const { return commandBuffer; }

		VkCommandBuffer commandBuffer;
		VkCommandPool pool;
	};

	class CommandPoolArray
	{
	public:
		CommandPoolArray(size_t size);		
		CommandPoolArray(const CommandPoolArray&) = delete;
		~CommandPoolArray();

		void Reset(int32_t i);

		VkCommandPool operator[](int32_t i) const { return pools[i]; }
		size_t size() const { return pools.size(); }

		std::vector<VkCommandPool> pools;
	};

	class CommandBufferArray
	{
	public:
		CommandBufferArray(const CommandPoolArray& pools);
		CommandBufferArray(const CommandBufferArray&) = delete;
		~CommandBufferArray();

		CommandBuffer& operator[](int32_t i) { return *(commandBuffers[i]); }
		const CommandBuffer& operator[](int32_t i) const { return *(commandBuffers[i]); }

		void Begin()
		{
			for (auto& c : commandBuffers)
				c->Begin();
		}
		void Begin(int32_t i)
		{
			commandBuffers[i]->Begin();
		}
		void End()
		{
			for (auto& c : commandBuffers)
				c->End();
		}
		void End(int32_t i)
		{
			commandBuffers[i]->End();
		}
		void Submit(int32_t index)
		{
			commandBuffers[index]->Submit();
		}

		auto begin() { return commandBuffers.begin(); }
		auto end() { return commandBuffers.end(); }
		auto begin() const { return commandBuffers.begin(); }
		auto end() const { return commandBuffers.end(); }

		std::vector<CommandBuffer*> commandBuffers;
	};

	class VulkanBuffer : public Buffer
	{
	public:
		VulkanBuffer(size_t size, VkBufferUsageFlags flags, VkMemoryPropertyFlags memory_flags);
		VulkanBuffer(const VulkanBuffer& other, size_t size, VkBufferUsageFlags flags, VkMemoryPropertyFlags memory_flags);
		VulkanBuffer(const VulkanBuffer&) = delete;
		~VulkanBuffer();

		virtual void Copy(Ref<Buffer> buf) override;
		virtual void* Map() override;
		virtual void UnMap() override;
		virtual void Flush(size_t size = VK_WHOLE_SIZE, size_t offset = 0) override;
		virtual void SetData(const void* data, size_t size = 0) override;
		virtual void SetDataMapped(const void* data, size_t size) override;

		operator VkBuffer() const { return buffer; }
		explicit operator VkDeviceMemory() const { return memory; }

		virtual size_t Size() const override { return size; }

	private:
		VkBuffer buffer;
		VkDeviceSize size;
		VkDeviceMemory memory;

		friend class VertexBuffer;
		friend class IndexBuffer;
	};

}