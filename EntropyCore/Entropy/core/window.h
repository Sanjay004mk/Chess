#pragma once
#include "Entropy/events/event.h"
#include "log.h"

struct GLFWwindow;

namespace et
{
	struct WindowProperties
	{
		uint32_t Width, Height;
		std::string Title;

		WindowProperties(const std::string& title = "Entropy", uint32_t width = 800, uint32_t height = 800)
			: Width(width), Height(height), Title(title)
		{}
	};
	class Window
	{
	public:
		using EventCallbackFn = std::function<void(Event&)>;

		Window(const WindowProperties& properties = WindowProperties());
		~Window();

		void OnUpdate();
		uint32_t GetWidth() const { return mData.Width; }
		uint32_t GetHeight() const { return mData.Height; }

		void SetEventCallbackFn(const EventCallbackFn& callback) { mData.callback = callback; }
		void SetVSync(bool enabled) { mData.VSync = enabled; }
		bool IsVSync() const { return mData.VSync; }

		GLFWwindow* GetNativeWindow() { return mpWindow; }

	private:
		void Init(const WindowProperties& properties);
		void Shutdown();

		GLFWwindow* mpWindow = nullptr;
		struct WindowData
		{
			uint32_t Width, Height;
			std::string Title;
			bool VSync;

			EventCallbackFn callback;
		};
		WindowData mData;
	};
}