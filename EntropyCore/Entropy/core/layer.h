#pragma once
#include <functional>
#include <string>

#include "log.h"
#include "Entropy/events/event.h"
#include "time_utils.h"

namespace et
{
	/// <summary>
	/// Base class. Used to derive and embed external application into Entropy
	/// </summary>
	class Layer
	{
	public:
		Layer(const std::string& LayerName = "Layer")
			: mLayerName(LayerName)
		{
			ET_LOG_TRACE("Creating Layer '{0}'", LayerName);
		}
		virtual ~Layer()
		{
			ET_LOG_TRACE("Destroying Layer '{0}'", mLayerName);
		}

		/// <summary>
		/// Called when the layer is added into et::Application
		/// </summary>
		virtual void OnAttach() = 0;

		/// <summary>
		/// Called when the layer is removed from et::Application
		/// </summary>
		virtual void OnDetach() = 0;

		/// <summary>
		/// Called every frame with time step
		/// </summary>
		/// <param name="delta">time step between frames</param>
		virtual void OnUpdate(TimeStep delta) {}

		/// <summary>
		/// Used to call ImGui functions
		/// </summary>
		virtual void OnImGuiRender() {}

		/// <summary>
		/// Override this function to get event callback
		/// </summary>
		/// <param name="event"></param>
		virtual void OnEvent(Event& event) {}

		const std::string& GetName() const { return mLayerName; }
	private:
		std::string mLayerName;
	};

	class ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();

		void Begin();
		void End();

		void OnAttach() override;
		void OnDetach() override;
		void OnUpdate(TimeStep delta) override;
		void OnEvent(Event& rEvent) override;
		void OnImGuiRender() override;
	private:
	};
}