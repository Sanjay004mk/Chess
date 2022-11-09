#include <etpch.h>
#include <queue>
#include <stb_image/stb_image.h>

#include "Entropy/scene/components.h"
#include "renderer.h"
#include "vk/vkapi.h"

namespace et
{
	RenderAPI* Renderer::spRenderAPI;

	static constexpr size_t MAX_QUADS = 100000;
	static uint32_t quadCount = 0;

	static Ref<VertexBuffer> vertexBuffer;
	static Vertex* vertices = nullptr;
	static Ref<IndexBuffer> indexBuffer;
	static uint32_t* indices = nullptr;

	static Ref<Pipeline> currentPipeline;
	static Ref<Renderpass> currentRenderpass;
	static Ref<Shader> currentShader;
	static std::queue<std::pair<Ref<Renderpass>, Ref<Framebuffer>>> renderpassQueue;
	static std::queue<Ref<Pipeline>> pipelineQueue;
	static std::queue<Ref<Shader>> shaderQueue;

	void Renderer::Init(RenderAPI::API api)
	{
		ET_LOG_TRACE("Renderer Initializing...");

		switch (api)
		{
		case RenderAPI::API::Vulkan:
			spRenderAPI = new VulkanAPI();
			break;
		}

		spRenderAPI->Init();
		vertexBuffer = CreateRef<VertexBuffer>(&vertices, MAX_QUADS * 4);
		indexBuffer = CreateRef<IndexBuffer>(&indices, MAX_QUADS * 6);
	}

	void Renderer::Shutdown()
	{
		vertexBuffer.reset();
		indexBuffer.reset();

		currentShader.reset();
		currentPipeline.reset();
		currentRenderpass.reset();
		pipelineQueue = std::queue<Ref<Pipeline>>();
		renderpassQueue = std::queue<std::pair<Ref<Renderpass>, Ref<Framebuffer>>>();
		shaderQueue = std::queue<Ref<Shader>>();
		ET_LOG_TRACE("Renderer Shutting down...");
		spRenderAPI->Shutdown(); 
		delete spRenderAPI;
	}

	void Renderer::BeginRenderpass(Ref<Renderpass> renderpass, Ref<Framebuffer> framebuffer)
	{
		currentRenderpass = renderpass;
		renderpass->Bind(framebuffer);
	}

	void Renderer::EndRenderpass()
	{
		currentRenderpass->UnBind();
		currentRenderpass.reset();
	}

	void Renderer::BindPipeline(Ref<Pipeline> pipeline)
	{
		currentPipeline = pipeline;
		pipeline->Bind();
	}

	void Renderer::UnBindPipeline()
	{
		currentPipeline->UnBind();
		currentPipeline.reset();
	}

	void Renderer::BindShader(Ref<Shader> shader)
	{
		currentShader = shader;
		shader->Bind();
	}

	void Renderer::UnBindShader()
	{
		currentShader->UnBind();
		currentShader.reset();
	}

	void Renderer::QueueRenderpass(Ref<Renderpass> renderpass, Ref<Framebuffer> framebuffer)
	{
		renderpassQueue.push({ renderpass, framebuffer });
	}

	void Renderer::QueuePipeline(Ref<Pipeline> pipeline)
	{
		pipelineQueue.push(pipeline);
	}

	void Renderer::QueueShader(Ref<Shader> shader)
	{
		shaderQueue.push(shader);
	}

	void Renderer::BeginNextRenderpass()
	{
		auto& [renderpass, framebuffer] = renderpassQueue.front();
		BeginRenderpass(renderpass, framebuffer);
		renderpassQueue.pop();
	}

	void Renderer::BindNextPipeline()
	{
		auto& pipeline = pipelineQueue.front();
		BindPipeline(pipeline);
		pipelineQueue.pop();
	}

	void Renderer::BindNextShader()
	{
		auto& shader = shaderQueue.front();
		BindShader(shader);
		shaderQueue.pop();
	}

	void Renderer::Flush()
	{
		if (quadCount == 0)
			return;

		vertexBuffer->count = quadCount * 4;
		indexBuffer->count = quadCount * 6;

		RenderCommand::DrawIndexed(*vertexBuffer, *indexBuffer);

		quadCount = 0;
		vertices = vertexBuffer->GetVertices();
		indices = indexBuffer->GetIndices();
	}

	void Renderer::DrawQuad(const Quad& quad)
	{
		DrawQuad(quad, -1);
	}

	void Renderer::DrawQuad(const Quad& quad, int32_t textureIndex)
	{
		if (quadCount >= MAX_QUADS)
			ET_ASSERT_MSG(false, "fix me!");

		{
			(*vertices).position = quad.position + quad.size / 2.f;
			(*vertices).color = quad.color;
			(*vertices).uv = quad.uvs[0];
			(*vertices).texture_index = textureIndex;

			vertices++;

			(*vertices).position = glm::vec2(quad.position.x + quad.size.x / 2.f, quad.position.y - quad.size.y / 2.f);
			(*vertices).color = quad.color;
			(*vertices).uv = quad.uvs[1];
			(*vertices).texture_index = textureIndex;

			vertices++;

			(*vertices).position = quad.position - quad.size / 2.f;
			(*vertices).color = quad.color;
			(*vertices).uv = quad.uvs[2];
			(*vertices).texture_index = textureIndex;

			vertices++;

			(*vertices).position = glm::vec2(quad.position.x - quad.size.x / 2.f, quad.position.y + quad.size.y / 2.f);
			(*vertices).color = quad.color;
			(*vertices).uv = quad.uvs[3];
			(*vertices).texture_index = textureIndex;

			vertices++;
		}

		{
			(*indices) = (quadCount * 4) + 0;
			indices++;
			(*indices) = (quadCount * 4) + 1;
			indices++;
			(*indices) = (quadCount * 4) + 2;
			indices++;
			(*indices) = (quadCount * 4) + 2;
			indices++;
			(*indices) = (quadCount * 4) + 3;
			indices++;
			(*indices) = (quadCount * 4) + 0;
			indices++;
		}

		quadCount++;
	}
}
