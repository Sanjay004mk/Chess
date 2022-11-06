#pragma once
#include <Entropy/EntropyScene.h>

namespace chs
{
	class TileManager
	{
	public:

		void* GetProjection() { return &camera.projection; }
		void DrawTiles();
		void SetCamera(uint32_t viewportWidth, uint32_t viewportHeight);

	private:
		et::OrthographicCamera camera;
	};
}