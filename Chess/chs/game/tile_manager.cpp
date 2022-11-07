#include "tile_manager.h"
#include "Entropy/Entropy.h"

namespace chs
{
	void TileManager::Load(std::string_view fen_string)
	{
		// start from top-left (vulkan -y up system)
		glm::vec2 position(1.f, 1.f);
		for (size_t i = 0; i < fen_string.size(); i++)
		{
			if (std::isalpha(fen_string[i]))
			{
				char lower = std::tolower(fen_string[i]);
				PieceType type = CharToPiece[lower];
				switch (type)
				{
#define CASE_TYPE(piece) case PieceType_##piece:\
								pieces[position] = et::CreateRef<piece>(CharToPiece[fen_string[i]]);\
								break

					CASE_TYPE(Pawn);
					CASE_TYPE(Rook);
					CASE_TYPE(Knight);
					CASE_TYPE(Bishop);
					CASE_TYPE(Queen);
					CASE_TYPE(King);

#undef CASE_TYPE
				}

				position.x++;
			}
			else if (fen_string[i] == ' ')
				break;
			else
			{
				if (std::isdigit(fen_string[i]))
					position.x += (float)(fen_string[i] - '0');

				// should be '/'
				else
				{
					position.x = 1.f;
					position.y++;
				}
			}
		}
	}

	void TileManager::DrawTiles()
	{
		et::Quad q;

		// board
		{
			for (float x = -3.5f; x < 4.f; x++)
			{
				for (float y = -3.5f; y < 4.f; y++)
				{
					q.position = { x, y };
					q.color = (int32_t)(x + y) % 2 == 0 ? glm::vec3(0.77, 0.82, 0.69) : glm::vec3(0.19, 0.35, 0.24);

					auto tilePos = q.position + glm::vec2(4.5f);
					if (tilePos == hoveredPiecePos)
						q.color -= glm::vec3(0.1f);

					et::Renderer::DrawQuad(q);
				}
			}
		}

		// pieces
		{
			q.color = glm::vec3(1.f);
			for (auto& [position, piece] : pieces)
			{
				if (piece == hoveredPiece || piece == clickedPiece)
					continue;

				q.position = position - glm::vec2(4.5f);
				int32_t texture_index = piece->type;

				et::Renderer::DrawQuad(q, texture_index);
			}
			auto drawPiece = [&](const glm::vec2& piecePos, const et::Ref<Piece>& piece, float intensity)
			{
				q.position = piecePos - glm::vec2(4.5f);
				q.color = glm::vec3(intensity);
				int32_t texture_index = piece->type;

				et::Renderer::DrawQuad(q, texture_index);
			};

			if (hoveredPiece)
			{
				drawPiece(hoveredPiecePos, hoveredPiece, 0.7f);
			}
			if (clickedPiece)
			{
				drawPiece(clickedPiecePos, clickedPiece, 0.8f);
			}
		}
	}

	void TileManager::SetCamera(uint32_t viewportWidth, uint32_t viewportHeight)
	{
		screenWidth = (float)viewportWidth;
		screenHeight = (float)viewportHeight;
		float aspectRatio = screenWidth / screenHeight;
		float width = 10.f, height = 10.f;

		if (aspectRatio >= 1.f)
			width = height * aspectRatio;
		else
			height = width / aspectRatio;

		camera.Reset(width, height, 2.f);
	}

	void TileManager::OnUpdate(et::TimeStep ts)
	{
		auto mousePos = et::Input::GetMousePosition();

		hoveredPiecePos = ScreenPosToTilePos(mousePos);

		auto it = pieces.find(hoveredPiecePos);
		if (it == pieces.end())
			hoveredPiece.reset();
		else 
			hoveredPiece = it->second;
	}

	void TileManager::OnMouseClick(const glm::vec2& mousePos)
	{
		clickedPos = ScreenPosToTilePos(mousePos);
	}

	void TileManager::OnMouseRelease(const glm::vec2& mousePos)
	{
		auto pos = ScreenPosToTilePos(mousePos);
		// mouse was released on the same tile it was clicked on
		if (pos == clickedPos)
		{
			auto it = pieces.find(pos);
			if (it == pieces.end())
				clickedPiece.reset();
			else
			{
				clickedPiece = it->second;
				clickedPiecePos = pos;
			}
		}
		else
		{
			// handle dragging and dropping pieces
		}
	}

	glm::vec2 TileManager::ScreenPosToTilePos(const glm::vec2& screenPos)
	{
		// convert to (1, 1)[top-left] -> (8, 8)[bottom-right] space
		auto mousePos = screenPos;

		mousePos.x -= screenWidth / 2.f;
		mousePos.x /= (screenWidth / camera.width);
		mousePos.x = std::floor(mousePos.x) + 0.5f;

		mousePos.y -= (screenHeight / 2.f);
		mousePos.y /= (screenHeight / camera.height);
		mousePos.y = std::floor(mousePos.y) + 0.5f;

		mousePos += glm::vec2(4.5f);

		return mousePos;
	}
}