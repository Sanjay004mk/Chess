#pragma once
#include "Entropy/EntropyUtils.h"
#include "render_info.h"

namespace et
{
	struct Vertex;

	struct BufferCreateInfo
	{
		size_t size = 0;
		BufferUsageFlags usageFlags = 0;
		MemoryPropertyFlags memoryPropertyFlags = 0;
	};
	
	class Buffer
	{
	public:
		Buffer() = default;
		Buffer(const Buffer&) = delete;
		virtual ~Buffer() {}

		virtual void Copy(Ref<Buffer> buf) = 0;
		virtual void* Map() = 0;
		virtual void UnMap() = 0;
		virtual void Flush(size_t size, size_t offset) = 0;
		virtual void SetData(const void* data, size_t size) = 0;
		virtual void SetDataMapped(const void* data, size_t size) = 0;
		virtual size_t Size() const = 0;

		void* data = nullptr;
		bool coherent = false;
	};

	class BufferArray
	{
	public:
		BufferArray(const BufferCreateInfo& createInfo, size_t arraySize);
		BufferArray(const BufferArray&) = delete;
		~BufferArray();

		void* Map(uint32_t array_index) { return buffers[array_index]->Map(); }
		void UnMap(uint32_t array_index) { buffers[array_index]->UnMap(); }
		void Flush(uint32_t array_index, size_t size = 0, size_t offset = 0) { buffers[array_index]->Flush(size, offset); }
		void SetData(uint32_t array_index, const void* data, size_t size = 0) { buffers[array_index]->SetData(data, size); }
		void SetDataMapped(uint32_t array_index, const void* data, size_t size = 0) { buffers[array_index]->SetDataMapped(data, size); }

		size_t Size() const { return buffers[0]->Size(); }

		Ref<Buffer> operator[](int32_t i) { return (buffers[i]); }
		const Ref<Buffer> operator[](int32_t i) const { return (buffers[i]); }

		auto begin() { return buffers.begin(); }
		auto end() { return buffers.end(); }
		auto begin() const { return buffers.begin(); }
		auto end() const { return buffers.end(); }

		std::vector<Ref<Buffer>> buffers;
	};

	Ref<Buffer> CreateBuffer(const BufferCreateInfo& createInfo);
	Ref<BufferArray> CreateBufferArray(const BufferCreateInfo& createInfo, size_t arraySize);

	class VertexBuffer
	{
	public:
		VertexBuffer(const std::vector<Vertex>& vertices);
		VertexBuffer(Vertex** vertices, size_t size);
		VertexBuffer(const VertexBuffer&) = delete;
		~VertexBuffer();

		void Bake();
		Ref<Buffer> Get() { return buffer; }

		Vertex* GetVertices() { return (Vertex*)stagingBuffer->data; }

		const size_t size;
		size_t count;
		Ref<Buffer> buffer;
		Ref<Buffer> stagingBuffer;
	};

	class IndexBuffer
	{
	public:
		IndexBuffer(const std::vector<uint32_t>& indices);
		IndexBuffer(uint32_t** indices, size_t size);
		IndexBuffer(const IndexBuffer&) = delete;
		~IndexBuffer();

		void Bake();
		Ref<Buffer> Get() { return buffer; }

		uint32_t* GetIndices() { return (uint32_t*)stagingBuffer->data; }

		const size_t size;
		size_t count;
		Ref<Buffer> buffer;
		Ref<Buffer> stagingBuffer;
	};
}