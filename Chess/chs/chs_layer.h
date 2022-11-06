#pragma once
#include <Entropy/Entropy.h>
#include "game/tile_manager.h"

namespace chs
{
	class ChessLayer : public et::Layer 
	{
	public:
		ChessLayer() = default;
		~ChessLayer() = default;

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnUpdate(et::TimeStep ts) override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(et::Event& e) override;

	private:
		void Resize(uint32_t newWidth, uint32_t newHeight);

		uint32_t width = 0, height = 0;

		TileManager tileManager;

		et::Ref<et::Texture> screen;
		et::Ref<et::Shader> defaultShader;
		et::Ref<et::Pipeline> pipeline;
		et::Ref<et::Renderpass> renderpass;
		et::Ref<et::Framebuffer> framebuffer;
	};
}