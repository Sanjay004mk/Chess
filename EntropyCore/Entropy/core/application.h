#pragma once
#include <vector>

#include "layer.h"
#include "window.h"
#include "Entropy/events/application_events.h"

#define CONFIG_FILE "Entropy.info"

namespace et
{
	struct ApplicationCommandLineArgs
	{
		int32_t argc = 0;
		char** argv = nullptr;

		const char* operator[](int32_t index)
		{
			ET_ASSERT_NO_MSG_BREAK(index < argc);
			return argv[index];
		}

		operator bool() const { return argc > 1; }
	};

	struct ApplicationSpecification
	{
		std::string ApplicationName = "Entropy Application";
		std::string WorkingDirectory = "";
		ApplicationCommandLineArgs CommandLineArgs;
	};

	/// <summary>
	/// Root class.
	/// Programs using Entropy can make a derived class from this and add layer to the layer stack in the constructor
	/// to embed the program into entropy and use its API.
	/// They must also provide a et::Application* CreateApplication(et::ApplicationCommandLinArgs) function where they return a pointer to the derived class object.
	/// The .cpp defining the function must include &lt;Entropy/EntryPoint.h&gt;
	/// </summary>
	class Application
	{
	public:
		Application(const ApplicationSpecification& specs) : mAppSpecs(specs) { spApplication = this; Init(); }
		virtual ~Application() { ShutDown(); }
		void Run();

		/// <summary>
		/// Handles window resizing and closing, also call OnEvent of all layers in the layer stack
		/// as long as the event is not handled
		/// </summary>
		void OnEvent(Event& e);

		/// <summary>
		/// Pass a heap allocated layer class derived object.
		/// The object will be deleted automatically when the application closes
		/// </summary>
		void PushLayer(Layer* pLayer);

		/// <summary>
		/// Closes the application after completing the current frame
		/// </summary>
		void Close() { mRunning = false; }

		Window& GetWindow() { return *mpWindow; }
		ApplicationSpecification& GetSpecification() { return mAppSpecs; }

		static Application& Get() { return *spApplication; }

	private:
		void Init();
		void Loop();
		void ShutDown();

		/// <summary>
		/// Remove a layer before the end of a application.
		/// Not recommended to call this function
		/// </summary>
		void PopLayer(Layer* pLayer);

		bool OnWindowClose(WindowCloseEvent& e);
		bool OnWindowResize(WindowResizeEvent& e);

		ApplicationSpecification mAppSpecs;
		Ref<Window> mpWindow;
		std::vector<Layer*> mLayers;
		Ref<ImGuiLayer> mpImGuiLayer;

		float mLastTime = 0.f;
		bool mRunning = true;
		bool mMinimized = false;

		static Application* spApplication;

		/// <summary>
		/// Function defined in user application. Should include &lt;Entropy/EntryPoint.h&gt; header file in the file it is defined in
		/// </summary>
		/// <param name="args">The command line arguements which are passed into the function</param>
		/// <returns>Returns a pointer to a heap allocated application class derived object</returns>
		friend Application* CreateApplication(ApplicationCommandLineArgs args);
	};
}