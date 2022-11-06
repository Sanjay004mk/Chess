#include <etpch.h>

#include "Entropy/scene/components.h"
#include "vkapi.h"

namespace et
{
	static VkCommandPool gPool;

	void InitCommandPool()
	{
		VkCommandPoolCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		info.queueFamilyIndex = VulkanAPI::GetQueueIndex();
		VkResult err = vkCreateCommandPool(VulkanAPI::GetDevice(), &info, nullptr, &gPool);
		ET_VK_ASSERT_MSG(err, "Failed to create command pool!");
	}

	void DestroyCommandPool()
	{
		vkDestroyCommandPool(VulkanAPI::GetDevice(), gPool, nullptr);
	}

	CommandBuffer::CommandBuffer(VkCommandPool commandPool)
	{
		pool = commandPool == VK_NULL_HANDLE ? gPool : commandPool;
		VkCommandBufferAllocateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		info.commandBufferCount = 1;
		info.commandPool = pool;
		info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		VkResult err = vkAllocateCommandBuffers(VulkanAPI::GetDevice(), &info, &commandBuffer);
		ET_VK_ASSERT_MSG(err, "Failed to allocate command buffer!");
	}

	CommandBuffer::~CommandBuffer()
	{
		vkFreeCommandBuffers(VulkanAPI::GetDevice(), pool, 1, &commandBuffer);
	}

	void CommandBuffer::Begin(VkCommandBufferUsageFlags flags)
	{
		VkCommandBufferBeginInfo info{};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		info.flags = flags;
		VkResult err = vkBeginCommandBuffer(commandBuffer, &info);
		ET_VK_ASSERT_MSG(err, "Failed to begin command buffer!");
	}

	void CommandBuffer::End()
	{
		VkResult err = vkEndCommandBuffer(commandBuffer);
		ET_VK_ASSERT_MSG(err, "Failed to end command buffer!");
	}

	void CommandBuffer::Submit()
	{
		VkSubmitInfo info{};
		info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		info.pCommandBuffers = &commandBuffer;
		info.commandBufferCount = 1;
		VkResult err = vkQueueSubmit(VulkanAPI::GetGraphicsQueue(), 1, &info, nullptr);
		ET_VK_ASSERT_MSG(err, "Failed to submit command buffer!");

		VulkanAPI::DeviceWaitIdle();
	}

	void CommandBuffer::Submit(VkSemaphore* pWaitSemaphores, uint32_t waitSemaphoresCount, VkPipelineStageFlags* pWaitDstStageFlags, VkSemaphore* pSignalSemaphores, uint32_t signalSemaphoreCount, VkFence fence)
	{
		VkSubmitInfo info{};
		info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		info.pCommandBuffers = &commandBuffer;
		info.commandBufferCount = 1;

		info.pSignalSemaphores = pSignalSemaphores;
		info.pWaitDstStageMask = pWaitDstStageFlags;
		info.pWaitSemaphores = pWaitSemaphores;
		info.signalSemaphoreCount = signalSemaphoreCount;
		info.waitSemaphoreCount = waitSemaphoresCount;

		VkResult err = vkQueueSubmit(VulkanAPI::GetGraphicsQueue(), 1, &info, fence);
		ET_VK_ASSERT_MSG(err, "Failed to submit command buffer!");
	}

	CommandPoolArray::CommandPoolArray(size_t size)
	{
		VkResult err;
		pools.resize(size);
		VkCommandPoolCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		info.queueFamilyIndex = VulkanAPI::GetQueueIndex();
		for (auto& pool : pools)
		{
			err = vkCreateCommandPool(VulkanAPI::GetDevice(), &info, nullptr, &pool);
			ET_VK_ASSERT_MSG(err, "Failed to create command pool!");
		}
	}

	CommandPoolArray::~CommandPoolArray()
	{
		for (auto& pool : pools)
			vkDestroyCommandPool(VulkanAPI::GetDevice(), pool, nullptr);
	}

	void CommandPoolArray::Reset(int32_t i)
	{
		VkResult err = vkResetCommandPool(VulkanAPI::GetDevice(), pools[i], 0);
	}

	CommandBufferArray::CommandBufferArray(const CommandPoolArray& pools)
	{
		commandBuffers.reserve(pools.size());
		for (size_t i = 0; i < pools.size(); i++)
			commandBuffers.emplace_back(new CommandBuffer(pools[(int32_t)i]));
	}

	CommandBufferArray::~CommandBufferArray()
	{
		for (auto& c : commandBuffers)
			delete c;
	}


	VulkanBuffer::VulkanBuffer(size_t size, VkBufferUsageFlags flags, VkMemoryPropertyFlags memory_flags)
		: size(size)
	{
		VkResult err;

		VkBufferCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		info.size = size;
		info.usage = flags;

		err = vkCreateBuffer(VulkanAPI::GetDevice(), &info, nullptr, &buffer);
		ET_VK_ASSERT_MSG(err, "Failed to create buffer!");

		VkMemoryRequirements requirements{};
		vkGetBufferMemoryRequirements(VulkanAPI::GetDevice(), buffer, &requirements);

		VkMemoryAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		alloc_info.allocationSize = requirements.size;
		alloc_info.memoryTypeIndex = VulkanAPI::GetMemoryTypeIndex(requirements.memoryTypeBits, memory_flags);

		err = vkAllocateMemory(VulkanAPI::GetDevice(), &alloc_info, nullptr, &memory);
		ET_VK_ASSERT_MSG(err, "Failed to allocate buffer memory!");

		err = vkBindBufferMemory(VulkanAPI::GetDevice(), buffer, memory, 0);
		ET_VK_ASSERT_MSG(err, "Failed to bind buffer memory!");

		if (coherent = (memory_flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
			data = Map();
	}

	VulkanBuffer::VulkanBuffer(const VulkanBuffer& other, size_t size, VkBufferUsageFlags flags, VkMemoryPropertyFlags memory_flags)
		: VulkanBuffer(size, flags, memory_flags)
	{
		CommandBuffer commandBuffer;
		VkBufferCopy region{};
		region.size = size;
		region.dstOffset = 0;
		region.srcOffset = 0;
		commandBuffer.Begin();
		vkCmdCopyBuffer(commandBuffer, other.buffer, buffer, 1, & region);
		commandBuffer.End();
		commandBuffer.Submit();
	}

	VulkanBuffer::~VulkanBuffer()
	{
		if (coherent)
			UnMap();

		vkDestroyBuffer(VulkanAPI::GetDevice(), buffer, nullptr);
		vkFreeMemory(VulkanAPI::GetDevice(), memory, nullptr);
	}

	void VulkanBuffer::Copy(Ref<Buffer> buf)
	{
		if (buf->Size() != Size())
		{
			ET_LOG_ERROR("size of buffer ({0}) is not the same ({1})", buf->Size(), Size());
			return;
		}
		VulkanBuffer& other = *(VulkanBuffer*)(buf.get());
		CommandBuffer commandBuffer;
		VkBufferCopy region{};
		region.size = size;
		region.dstOffset = 0;
		region.srcOffset = 0;
		commandBuffer.Begin();
		vkCmdCopyBuffer(commandBuffer, other.buffer, buffer, 1, &region);
		commandBuffer.End();
		commandBuffer.Submit();
	}

	void* VulkanBuffer::Map()
	{
		void* data;
		vkMapMemory(VulkanAPI::GetDevice(), memory, 0, size, 0, &data);
		return data;
	}

	void VulkanBuffer::UnMap()
	{
		vkUnmapMemory(VulkanAPI::GetDevice(), memory);
	}

	void VulkanBuffer::Flush(size_t _size, size_t offset)
	{
		VkMappedMemoryRange range{};
		range.sType = VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE;
		range.memory = memory;
		range.offset = offset;
		range.size = _size;
		vkFlushMappedMemoryRanges(VulkanAPI::GetDevice(), 1, &range);
	}

	void VulkanBuffer::SetData(const void* data, size_t size)
	{
		ET_ASSERT_MSG_BREAK(size <= this->size, "data block larger than buffer size!");
		if (size == 0)
			size = this->size;
		memcpy(Map(), data, size);
		UnMap();
	}

	void VulkanBuffer::SetDataMapped(const void* data, size_t size)
	{
		ET_ASSERT_MSG_BREAK(size <= this->size, "data block larger than buffer size!");
		if (size == 0)
			size = this->size;
		memcpy(this->data, data, size);
	}

	BufferArray::BufferArray(const BufferCreateInfo& createInfo, size_t arraySize)
	{
		buffers.reserve(arraySize);
		for (size_t i = 0; i < arraySize; i++)
			buffers.emplace_back(CreateRef<VulkanBuffer>(createInfo.size, (VkBufferUsageFlags)createInfo.usageFlags, (VkMemoryPropertyFlags)createInfo.memoryPropertyFlags));
	}

	BufferArray::~BufferArray()
	{
		buffers.clear();
	}

	VertexBuffer::VertexBuffer(const std::vector<Vertex>& vertices)
		: size(vertices.size())
	{
		size_t size = count * sizeof(Vertex);
		stagingBuffer = CreateRef<VulkanBuffer>(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		stagingBuffer->SetDataMapped(vertices.data(), size);

		buffer = CreateRef<VulkanBuffer>(*(VulkanBuffer*)stagingBuffer.get(), size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	}

	VertexBuffer::VertexBuffer(Vertex** vertices, size_t size)
		: size(size)
	{
		size *= sizeof(Vertex);
		stagingBuffer = CreateRef<VulkanBuffer>(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		*vertices = GetVertices();

		buffer = CreateRef<VulkanBuffer>(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	}

	VertexBuffer::~VertexBuffer()
	{
	}

	void VertexBuffer::Bake()
	{
		buffer->Copy(stagingBuffer);
	}

	IndexBuffer::IndexBuffer(const std::vector<uint32_t>& indices)
		: size(indices.size())
	{
		size_t size = count * sizeof(uint32_t);
		stagingBuffer = CreateRef<VulkanBuffer>(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		stagingBuffer->SetDataMapped(indices.data(), size);

		buffer = CreateRef<VulkanBuffer>(*(VulkanBuffer*)stagingBuffer.get(), size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	}

	IndexBuffer::IndexBuffer(uint32_t** indices, size_t size)
		: size(size)
	{
		size *= sizeof(uint32_t);
		stagingBuffer = CreateRef<VulkanBuffer>(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		*indices = GetIndices();

		buffer = CreateRef<VulkanBuffer>(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	}

	IndexBuffer::~IndexBuffer()
	{
	}

	void IndexBuffer::Bake()
	{
		buffer->Copy(stagingBuffer);
	}

	Ref<Buffer> CreateBuffer(const BufferCreateInfo& createInfo)
	{
		return CreateRef<VulkanBuffer>(createInfo.size, (VkBufferUsageFlags)createInfo.usageFlags, (VkMemoryPropertyFlags)createInfo.memoryPropertyFlags);
	}

	Ref<BufferArray> CreateBufferArray(const BufferCreateInfo& createInfo, size_t arraySize)
	{
		return CreateRef<BufferArray>(createInfo, arraySize);
	}

}