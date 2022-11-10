#include "Entropy/Entropy.h"

#include "board.h"
#include "tile_manager.h"

#define MOVE_TILE_TEXTURE_INDEX 0

namespace chs
{

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
						q.color -= glm::vec3(0.17f);
					if (tilePos == clickedPiecePos)
						q.color -= glm::vec3(0.19f);

					et::Renderer::DrawQuad(q);
				}
			}
		}
		// pieces
		if (board)
		{
			q.color = glm::vec3(1.f);
			auto& b = *board;
			for (auto piece : b)
			{
				if (piece.position == clickedPos && dragPiece)
					continue;
				q.position = piece.position - glm::vec2(4.5f);
				et::Renderer::DrawQuad(q, piece.type);
			}
			if (dragPiece)
			{
				q.position = mousePos + clickedPieceDragOffset;
				et::Renderer::DrawQuad(q, dragPiece);
			}

			// captured pieces
			q.size = glm::vec2(0.5f);
			glm::vec2 blackPos = glm::vec2(4.25f, -3.75f);
			glm::vec2 whitePos = glm::vec2(4.25f, 3.75f);
			for (const auto& [index, type] : b.CapturedPieces())
			{
				if (IsWhite(type))
				{
					q.position = whitePos;
					if (whitePos.y < 0.5f)
					{
						whitePos.y = 3.75f;
						whitePos.x = 4.75f;
					}
					else
						whitePos.y -= 0.5f;

					et::Renderer::DrawQuad(q, type);
				}
				else
				{
					q.position = blackPos;
					if (blackPos.y > -0.5f)
					{
						blackPos.y = -3.75f;
						blackPos.x = 4.75f;
					}
					else
						blackPos.y += 0.5f;

					et::Renderer::DrawQuad(q, type);
				}
			}
			q.size = glm::vec2(1.f);
		}

		// highlight moveTiles
		{
			q.color = glm::vec3(1.f);
			for (auto& [tile, move] : moveTiles)
			{
				q.position = tile - glm::vec2(4.5f);

				et::Renderer::DrawQuad(q, MOVE_TILE_TEXTURE_INDEX);
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
		hoveredPiecePos = ScreenPosToTilePos(et::Input::GetMousePosition());
		mousePos = ScreenPosToWorldPos(et::Input::GetMousePosition());
	}

	void TileManager::OnMouseClick(const glm::vec2& mousePos)
	{
		clickedPos = ScreenPosToTilePos(mousePos);
		clickedPieceDragOffset = (clickedPos - glm::vec2(4.5f)) - ScreenPosToWorldPos(mousePos);
		dragPiece = board->GetTile(clickedPos);
		if (moveTiles.count(clickedPos) == 0)
		{
			shouldMove = false;
			clickedPiecePos = clickedPos;
			moveTiles.clear();
			if (board->IsPiece(clickedPos))
				moveTiles = board->GetMoveTiles(clickedPiecePos);
		}
		else
			shouldMove = true;
	}

	void TileManager::OnMouseRelease(const glm::vec2& mousePos)
	{
		auto pos = ScreenPosToTilePos(mousePos);
		if (pos != clickedPos || shouldMove)
		{
			if (moveTiles.count(pos) != 0)
				board->MovePiece(moveTiles.at(pos));
			moveTiles.clear();
		}
		dragPiece = 0;
	}

	glm::vec2 TileManager::ScreenPosToTilePos(const glm::vec2& screenPos)
	{
		auto mousePos = ScreenPosToWorldPos(screenPos);
		mousePos.x = std::floor(mousePos.x) + 0.5f;
		mousePos.y = std::floor(mousePos.y) + 0.5f;

		mousePos += glm::vec2(4.5f);

		return mousePos;
	}

	glm::vec2 TileManager::ScreenPosToWorldPos(const glm::vec2& screenPos)
	{
		// convert to (1, 1)[bottom-left] -> (8, 8)[top-right] space
		auto mousePos = screenPos;

		mousePos.x -= screenWidth / 2.f;
		mousePos.x /= (screenWidth / camera.width);

		mousePos.y = (screenHeight - mousePos.y);
		mousePos.y -= (screenHeight / 2.f);
		mousePos.y /= (screenHeight / camera.height);

		return mousePos;
	}

	glm::vec2 TileManager::WorldPosToScreenPos(const glm::vec2& worldPos)
	{
		auto mousePos = worldPos;

		mousePos.x *= (screenWidth / camera.width);
		mousePos.x += screenWidth / 2.f;

		mousePos.y *= (screenHeight / camera.height);
		mousePos.y += (screenHeight / 2.f);
		mousePos.y = (screenHeight - mousePos.y);

		return mousePos;
	}

	float TileManager::WorldToScreenUnit()
	{
		return (screenWidth / camera.width);
	}

}