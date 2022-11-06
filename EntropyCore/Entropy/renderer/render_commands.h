#pragma once
#include "Entropy/EntropyUtils.h"
#include "render_info.h"

namespace et
{
	class Renderpass;
	class Shader;
	class Buffer;
	class Texture;
	class Framebuffer;
	class Pipeline;
	class VertexBuffer;
	class IndexBuffer;
	class CommandBuffer;
	class CommandBufferArray;

	constexpr uint32_t maxCommandBuffers = 8;

	class RenderCommand
	{
	public:
		static void Init();
		static void Shutdown();

		static void Begin();
		static void End();

		static CommandBuffer* GetCommandBuffer();
		
		static void SetClearColor(const glm::vec4& clearColor, uint32_t attachment);
		static void SetClearColor(const glm::ivec4& r32ClearColor, uint32_t attachment);
		static void SetDepthClearColor(float depthClearColor, uint32_t attachment);
		static void SetStencilClearColor(uint32_t stencilClearColor, uint32_t attachment);
		static void ResetClearColor();
		
		static void DrawIndexed(const VertexBuffer& vertexBuffer, const IndexBuffer& indexBuffer);
		static void Draw(const VertexBuffer& vertexBuffer);
		static void Draw(uint32_t count);
		
		static void StartCommandBuffer();
		static void EndCommandBuffer();
	};
}