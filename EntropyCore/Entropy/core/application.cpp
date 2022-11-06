#include <etpch.h>

#include "application.h"
#include "Entropy/events/mouse_events.h"
#include "Entropy/renderer/renderer.h"
#include "time_utils.h"
#include "Entropy/renderer/vk/vkapi.h"

namespace et
{
	et::Application* Application::spApplication = nullptr;

	void Application::Run()
	{
		Loop();
	}

	void Application::Init()
	{
		ET_LOG_TRACE("Application starting...");

		mpWindow = CreateRef<Window>(mAppSpecs.ApplicationName);
		mpWindow->SetEventCallbackFn([this](Event& e) { return this->OnEvent(e); });

		if (!mAppSpecs.WorkingDirectory.empty())
			std::filesystem::current_path(mAppSpecs.WorkingDirectory);

		Renderer::Init(RenderAPI::API::Vulkan);

		mpImGuiLayer = CreateRef<ImGuiLayer>();
		mpImGuiLayer->OnAttach();
	}

	void Application::ShutDown()
	{
		ET_LOG_TRACE("Application shutting down...");

		VulkanAPI::DeviceWaitIdle();

		std::ofstream file(CONFIG_FILE);
		file.close();

		for (auto& pLayer : mLayers)
			PopLayer(pLayer);

		mpImGuiLayer->OnDetach();
		mpImGuiLayer.reset();

		Renderer::Shutdown();
		mpWindow.reset();
	}

	void Application::Loop()
	{
		while (mRunning)
		{
			Timer t;
			TimeStep ts = mLastTime;
			Time::sDelta = mLastTime;

			if (!mMinimized)
			{
				Renderer::BeginFrame();

				for (Layer* pLayer : mLayers)
					pLayer->OnUpdate(ts);


				mpImGuiLayer->Begin();
				for (Layer* pLayer : mLayers)
					pLayer->OnImGuiRender();
				mpImGuiLayer->End();

				Renderer::EndFrame();
			}
			mpWindow->OnUpdate();
			mLastTime = t.Elapsed();
		}
	}

	void Application::OnEvent(Event& rEvent)
	{
		EventDispatcher dispatcher(rEvent);
		dispatcher.Dispatch<WindowCloseEvent>([this](WindowCloseEvent& e) { return this->OnWindowClose(e); });
		dispatcher.Dispatch<WindowResizeEvent>([this](WindowResizeEvent& e) { return this->OnWindowResize(e); });

		for (auto it = mLayers.rbegin(); it != mLayers.rend(); ++it)
		{
			if (rEvent.handled)
				break;

			(*it)->OnEvent(rEvent);
		}
	}

	void Application::PushLayer(Layer* pLayer)
	{
		mLayers.emplace_back(pLayer);
		pLayer->OnAttach();
	}

	void Application::PopLayer(Layer* pLayer)
	{
		pLayer->OnDetach();
		auto it = std::find(mLayers.begin(), mLayers.end(), pLayer);
		mLayers.erase(it);
		delete pLayer;
	}

	bool Application::OnWindowClose(WindowCloseEvent& rEvent)
	{
		mRunning = false;
		return true;
	}

	bool Application::OnWindowResize(WindowResizeEvent& rEvent)
	{
		if (rEvent.GetWidth() == 0 || rEvent.GetHeight() == 0)
		{
			mMinimized = true;
			return false;
		}
		Renderer::Resize();
		mMinimized = false;
		return false;
	}
}