#pragma once
#include <Entropy/EntropyScene.h>
#include "pieces.h"

namespace chs
{
	class TileManager
	{
	public:
		void Load(std::string_view fen_string);

		void OnMouseClick(const glm::vec2& mousePos);
		void OnMouseRelease(const glm::vec2& mousePos);
		void OnUpdate(et::TimeStep ts);

		void* GetProjection() { return &camera.projection; }
		void DrawTiles();
		void SetCamera(uint32_t viewportWidth, uint32_t viewportHeight);

		// tmp
		glm::vec2 GetHoveredPiecePos() const { return hoveredPiecePos; }

	private:
		glm::vec2 ScreenPosToTilePos(const glm::vec2& screenPos);
		void GetLegalMoves();
		void MovePiece(const glm::vec2& pos);

		bool dragging = false;
		bool clickedOnce = false;
		bool clickedTwice = false;

		bool blacksTurn = false;

		float screenWidth, screenHeight;
		et::OrthographicCamera camera;

		std::vector<glm::vec2> moveTiles;
		glm::vec2 emPassant;
		std::array<glm::vec2, 2> castles;

		std::unordered_map<glm::vec2, et::Ref<Piece>> pieces;

		std::vector<et::Ref<Piece>> capturedPieces;

		et::Ref<Piece> hoveredPiece;
		glm::vec2 hoveredPiecePos;

		et::Ref<Piece> clickedPiece;
		glm::vec2 clickedPiecePos;

		glm::vec2 clickedPos;
	};
}