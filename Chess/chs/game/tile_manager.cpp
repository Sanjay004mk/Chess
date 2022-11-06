#include "tile_manager.h"
#include "Entropy/Entropy.h"

namespace chs
{
	void TileManager::DrawTiles()
	{
		for (float x = -3.5f; x < 4.f; x++)
		{
			for (float y = -3.5f; y < 4.f; y++)
			{
				et::Quad q;
				q.position = { x, y };
				q.color = (int32_t)(x + y) % 2 == 0 ? glm::vec3(0.97, 0.66, 0.79) : glm::vec3(0.15, 0.12, 0.14);

				et::Renderer::DrawQuad(q);
			}
		}
	}

	void TileManager::SetCamera(uint32_t viewportWidth, uint32_t viewportHeight)
	{
		float aspectRatio = (float)viewportWidth / (float)viewportHeight;
		float width = 8.f, height = 8.f;

		if (aspectRatio >= 1.f)
			width = height * aspectRatio;
		else
			height = width / aspectRatio;

		camera.Reset(width, height, 2.f);
	}
}