#pragma once
#include <Entropy/EntropyScene.h>
#include "pieces.h"

namespace chs
{
	class Board;
	struct Move;
	struct PieceRenderInfo;

	class TileManager
	{
	public:
		void OnMouseClick(const glm::vec2& mousePos);
		void OnMouseRelease(const glm::vec2& mousePos);
		void OnUpdate(et::TimeStep ts);

		void* GetProjection() { return &camera.projection; }
		void DrawTiles();
		void SetCamera(uint32_t viewportWidth, uint32_t viewportHeight);
		void ClearTiles() { moveTiles.clear(); clickedPiecePos = glm::vec2(0.f); }
		void UpdateFromTo(const glm::vec2& from, const glm::vec2& to) { lastFrom = from; lastTo = to; }

		bool Promote(PieceType piece);

		glm::vec2 ScreenPosToTilePos(const glm::vec2& screenPos);
		glm::vec2 ScreenPosToWorldPos(const glm::vec2& screenPos);
		glm::vec2 WorldPosToScreenPos(const glm::vec2& worldPos);
		float WorldToScreenUnit();

		Board* board;

	private:
		void Animate();

		std::vector<PieceRenderInfo> pieces;
		glm::vec2 lastFrom = {}, lastTo = {};

		float screenWidth, screenHeight;
		et::OrthographicCamera camera;

		bool shouldMove = false;
		std::unordered_map<glm::vec2, Move> moveTiles;

		glm::vec2 hoveredPiecePos;
		glm::vec2 clickedPiecePos;
		glm::vec2 clickedPos;
		glm::vec2 clickedPieceDragOffset;
		glm::vec2 mousePos;
		PieceType dragPiece = 0;
	};
}
