#pragma once
#include <Entropy/EntropyScene.h>
#include "pieces.h"

namespace chs
{
	class TileManager
	{
	public:
		void Load(std::string_view fen_string);

		void* GetProjection() { return &camera.projection; }
		void DrawTiles();
		void SetCamera(uint32_t viewportWidth, uint32_t viewportHeight);

	private:

		et::OrthographicCamera camera;

		std::vector<et::Ref<Piece>> pieces;
	};
}