#pragma once
#include <ImGui/imgui.h>
#include <glm/glm.hpp>

#include "Entropy/EntropyUtils.h"
#include "render_info.h"
#include "renderer_api.h"
#include "render_commands.h"
#include "buffer.h"
#include "pipeline.h"
#include "render_pass.h"
#include "shader.h"
#include "texture.h"

namespace et
{
	struct Quad;

	class Renderer
	{
	public:
		static void Init(RenderAPI::API api);
		static void ImGuiInit() { spRenderAPI->ImGuiInit(); }
		static void Shutdown();
		static void ImGuiShutdown() { spRenderAPI->ImGuiShutdown(); }

		static void BeginFrame() { spRenderAPI->BeginFrame(); }
		static void EndFrame() { spRenderAPI->EndFrame(); }

		static void ImGuiBegin() { spRenderAPI->ImGuiBegin(); }
		static void ImGuiEnd() { spRenderAPI->ImGuiEnd(); }

		static void Resize() { spRenderAPI->Resize(); }
		static TextureFormat GetSurfaceFormat() { return spRenderAPI->GetSurfaceFormat(); }

		static void Present(const Ref<Texture>& image) { spRenderAPI->Present(image); }

		static void Flush();

		static void BeginRenderpass(Ref<Renderpass> renderpass, Ref<Framebuffer> framebuffer);
		static void EndRenderpass();
		static void BindPipeline(Ref<Pipeline> pipeline);
		static void UnBindPipeline();
		static void BindShader(Ref<Shader> shader);
		static void UnBindShader();

		static void QueueRenderpass(Ref<Renderpass> renderpass, Ref<Framebuffer> framebuffer);
		static void QueuePipeline(Ref<Pipeline> pipeline);
		static void QueueShader(Ref<Shader> shader);
		static void BeginNextRenderpass();
		static void BindNextPipeline();
		static void BindNextShader();

		static void DrawQuad(const Quad& q);
		static void DrawQuad(const Quad& q, int32_t textureIndex);
		
		static RenderAPI::API GetAPI() { return spRenderAPI->GetAPI(); }

	private:
		static RenderAPI* spRenderAPI;
	};
}