#pragma once
#include <Entropy/Entropy.h>
#include "game/tile_manager.h"
#include "game/board.h"

#include "ext/miniaudio.h"

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
		void StartNewGame();
		bool StartGame(std::string_view fen_string);
		void Resize(uint32_t newWidth, uint32_t newHeight);
		void DisplayBoardUI();
		void DisplayMainMenu();
		void DisplayVsPlayerMenu();
		void DisplayVsCompMenu();
		void DisplayLevelSelectMenu();
		void DisplayHomeButton();
		void PlayEngineMove();
		void NotifySearchComplete();
		void OfferHint();

		uint32_t width = 0, height = 0;

		enum class MenuState { MainMenu, VsComputer, VsPlayer, LevelSelect, InGame };
		MenuState menuState = MenuState::MainMenu;
		BoardSpecification boardCreateSpecs;

		TileManager tileManager;
		et::Ref<Board> board;

		et::Ref<et::Texture> screen;
		et::Ref<et::Shader> defaultShader;
		et::Ref<et::Pipeline> pipeline;
		et::Ref<et::Renderpass> renderpass;
		et::Ref<et::Framebuffer> framebuffer;

		std::vector<et::Ref<et::Texture>> textures;

		// audio
		friend void data_callback(ma_device* pDevice, void* pOutput, const void* pInput, ma_uint32 frameCount);
		ma_decoder place_piece;
		ma_decoder check;
		ma_decoder checkmate;
		ma_decoder* sound = nullptr;
		ma_device device;
	};
}