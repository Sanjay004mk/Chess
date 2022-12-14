#include "Entropy/Entropy.h"

#include "board.h"
#include "animator.h"
#include "tile_manager.h"

#define MOVE_TILE_TEXTURE_INDEX 0

namespace chs
{

	void TileManager::DrawTiles()
	{
		static et::Quad q;
		static const glm::vec3 lightTile = glm::vec3(0.77f, 0.82f, 0.69f);
		static const glm::vec3 darkTile = glm::vec3(0.22f, 0.35f, 0.24f);

		// board
		{
			for (float x = -3.5f; x < 4.f; x++)
			{
				for (float y = -3.5f; y < 4.f; y++)
				{
					q.position = { x, y };
					q.color = (int32_t)(x + y) % 2 == 0 ? lightTile : darkTile;

					auto tilePos = q.position + glm::vec2(4.5f);
					if (tilePos == hoveredPiecePos)
						q.color -= glm::vec3(0.17f);
					if (tilePos == clickedPiecePos)
						q.color -= glm::vec3(0.19f);

					et::Renderer::DrawQuad(q);
				}
			}
		}

		// highlight moveTiles
		{
			q.size = glm::vec2(.81f);
			// highlight last played move
			{
				q.color = glm::vec3(0.95f, 0.929f, 0.893f);
				if (InsideBoard(lastFrom))
				{
					q.position = lastFrom - glm::vec2(4.5f);
					et::Renderer::DrawQuad(q);
				}
				if (InsideBoard(lastTo))
				{
					q.position = lastTo - glm::vec2(4.5f);
					et::Renderer::DrawQuad(q);
				}

			}

			q.size = glm::vec2(.7f);
			for (auto& [tile, move] : moveTiles)
			{
				q.position = tile - glm::vec2(4.5f);
				q.color = (int32_t)(tile.x + tile.y) % 2 == 0 ? lightTile : darkTile;
				q.color -= glm::vec3(.2f);

				et::Renderer::DrawQuad(q);
			}
			q.size = glm::vec2(1.f);
		}


		// pieces
		if (board)
		{
			Animate();
			q.color = glm::vec3(1.f);
			for (auto& piece : pieces)
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
			for (const auto& [index, type] : board->CapturedPieces())
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
	}

	void TileManager::Animate()
	{
		auto& b = *board;
		pieces.clear();
		for (auto piece : b)
		{
			pieces.push_back(piece);
			// piece is animated
			if (pieceAnimations.count(piece.position))
				if (!Animator::Animate(pieces.back().position, pieceAnimations.at(piece.position)))
				{
					pieceAnimations.erase(piece.position);
					PlayMovePieceSound();
					// piece moving sound doesn't get played
					if (board->inCheck)
					{
						if (board->checkmate)
							PlayCheckMateSound();
						else
							PlayCheckSound();
					}
				}
		}
	}

	void TileManager::PlayAnimation(Move move)
	{
		PlayAnimation(GetPositionFromIndex(move.From()), GetPositionFromIndex(move.To()));
		if (move.Castle())
		{
			auto [from, to] = GetCastleTiles(move);
			PlayAnimation(from, to);
		}
	}

	void TileManager::PlayAnimation(const glm::vec2& from, const glm::vec2& to)
	{
		//   use 'to' because piece has already moved
		pieceAnimations[to] = Animator::Play(AnimationInfo(from, to, 0.4f));
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
		if (board && canMakeMove)
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
	}

	extern bool askPromotion = false;
	extern bool moveMade = false;

	void TileManager::OnMouseRelease(const glm::vec2& mousePos)
	{
		if (board && canMakeMove)
		{
			auto pos = ScreenPosToTilePos(mousePos);
			if ((pos != clickedPos || shouldMove) && !askPromotion)
			{
				if (moveTiles.count(pos) != 0)
				{
					auto& move = moveTiles.at(pos);
					if (board->MakeMove(move, askPromotion))
					{
						moveMade = true;
						UpdateFromTo(GetPositionFromIndex(move.From()), GetPositionFromIndex(move.To()));
						// piece wasn't dragged
						if (shouldMove)
							// 'Animate()' plays move piece sound when the animation ends
							PlayAnimation(move);
						else
						{
							PlayMovePieceSound();
							// piece moving sound doesn't get played
							if (board->inCheck)
							{
								if (board->checkmate)
									PlayCheckMateSound();
								else
									PlayCheckSound();
							}
						}
					}
				}
				moveTiles.clear();
				clickedPiecePos = glm::vec2(0.f);
			}

			dragPiece = 0;
		}
	}

	bool TileManager::Promote(PieceType piece) 
	{
		askPromotion = false;
		if (!board->Promote(piece))
			return false;

		if (board->inCheck)
		{
			if (board->checkmate)
				PlayCheckMateSound();
			else
				PlayCheckSound();
		}

		return true;
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