#include <etpch.h>
#include <ImGui/imgui_internal.h>
#include <ImGui/imgui.h>

#include "application.h"
#include "Entropy/ext/data.h"
#include "Entropy/renderer/renderer.h"
#include "layer.h"

#include "Entropy/renderer/vk/vkapi.h"

namespace et
{
	ImGuiLayer::ImGuiLayer()
        : Layer("ImGuiLayer")
	{
	}

	ImGuiLayer::~ImGuiLayer()
	{
	}

	void ImGuiLayer::OnAttach()
	{
		ET_LOG_TRACE("Renderer Initializing ImGui...");
		// Setup Dear ImGui context
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
		//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;           // Enable Docking
		io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;         // Enable Multi-Viewport / Platform Windows
		//io.ConfigViewportsNoAutoMerge = true;
		//io.ConfigViewportsNoTaskBarIcon = true;

		//ImGui::DockSpace(ImGui::GetID("MyDockspace"));

		// Setup Dear ImGui style
		ImGui::StyleColorsDark();
		//ImGui::StyleColorsLight();

		// When viewports are enabled we tweak WindowRounding/WindowBg so platform windows can look identical to regular ones.
		ImGuiStyle& style = ImGui::GetStyle();
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			style.WindowRounding = 0.0f;
			style.Colors[ImGuiCol_WindowBg].w = 1.0f;
		}

		ImFontConfig fontConfig;
		fontConfig.GlyphExtraSpacing = ImVec2(1.f, 0.f);
		fontConfig.FontDataOwnedByAtlas = false;
		ImFont* RobotoRegular = io.Fonts->AddFontFromMemoryTTF((void*)gRobotoRegular.data(), (int32_t)(gRobotoRegular.size() * sizeof(uint8_t)), 30.0f, &fontConfig);
		ImFont* CascadiaMono = io.Fonts->AddFontFromMemoryTTF((void*)gCascadiaMono.data(), (int32_t)(gCascadiaMono.size() * sizeof(uint8_t)), 18.0f, &fontConfig);
		io.FontDefault = RobotoRegular;

		auto& colors = ImGui::GetStyle().Colors;

		colors[ImGuiCol_WindowBg] = ImVec4{ 0.1f, 0.1f, 0.1f, 1.0f };

		// Headers
		colors[ImGuiCol_Header] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_HeaderHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_HeaderActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Buttons
		colors[ImGuiCol_Button] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_ButtonHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_ButtonActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Frame BG
		colors[ImGuiCol_FrameBg] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };
		colors[ImGuiCol_FrameBgHovered] = ImVec4{ 0.3f, 0.305f, 0.31f, 1.0f };
		colors[ImGuiCol_FrameBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// Tabs
		colors[ImGuiCol_Tab] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TabHovered] = ImVec4{ 0.38f, 0.3805f, 0.381f, 1.0f };
		colors[ImGuiCol_TabActive] = ImVec4{ 0.28f, 0.2805f, 0.281f, 1.0f };
		colors[ImGuiCol_TabUnfocused] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TabUnfocusedActive] = ImVec4{ 0.2f, 0.205f, 0.21f, 1.0f };

		// Title
		colors[ImGuiCol_TitleBg] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TitleBgActive] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };
		colors[ImGuiCol_TitleBgCollapsed] = ImVec4{ 0.15f, 0.1505f, 0.151f, 1.0f };

		// disable imgui.ini ( for chs )
		io.IniFilename = nullptr;

        Renderer::ImGuiInit();
	}

	void ImGuiLayer::OnDetach()
	{
        Renderer::ImGuiShutdown();
	}

	void ImGuiLayer::OnUpdate(TimeStep delta)
	{

	}

	void ImGuiLayer::OnEvent(Event& e)
	{
        ImGuiIO& io = ImGui::GetIO();
        e.handled |= e.IsInCategory(EventCategories::Mouse) & io.WantCaptureMouse;
        e.handled |= e.IsInCategory(EventCategories::Keyboard) & io.WantCaptureKeyboard;
	}

    void ImGuiLayer::Begin()
    {
        Renderer::ImGuiBegin();
        ImGui::NewFrame();
    }

    void ImGuiLayer::End()
    {
        Renderer::ImGuiEnd();
    }

	// et::Renderer::Present() image gets displayed here
	void ImGuiLayer::OnImGuiRender()
	{
		if (VulkanAPI::presentImage)
		{
			ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoDecoration;

			{
				ImGuiViewport* viewport = ImGui::GetMainViewport();
				ImVec2 size = viewport->Size;
				ImVec2 pos = viewport->Pos;
				ImGui::SetNextWindowPos(pos);
				ImGui::SetNextWindowSize(size);
				ImGui::SetNextWindowViewport(viewport->ID);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
				window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
				window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
				
				ImGui::Begin("viewport", nullptr, window_flags);
				ImGui::Image(VulkanAPI::presentImage->GetImGuiTextureID(), size, ImVec2(0, 1), ImVec2(1, 0));
				ImGui::End();

				ImGui::PopStyleVar(3);
			}

		}
	}
}