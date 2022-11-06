#pragma once
#include "render_info.h"

namespace et
{
	class Texture;

	class RenderAPI
	{
	public:
		enum class API { Vulkan };

		virtual ~RenderAPI() {}

		virtual API GetAPI() const = 0;

		virtual TextureFormat GetSurfaceFormat() const = 0;

		virtual void Init() = 0;
		virtual void ImGuiInit() = 0;
		virtual void Shutdown() = 0;
		virtual void ImGuiShutdown() = 0;
		virtual void BeginFrame() = 0;
		virtual void ImGuiBegin() = 0;
		virtual void EndFrame() = 0;
		virtual void ImGuiEnd() = 0;

		virtual void Resize() = 0;

		virtual void Present(Ref<Texture> image) = 0;
	};
}
