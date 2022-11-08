#include "pieces.h"

#if 0

namespace chs
{
	std::unordered_map<char, PieceType> CharToPiece =
	{
		{ 'p', PieceType_Pawn + PieceType_Black },
		{ 'r', PieceType_Rook + PieceType_Black },
		{ 'b', PieceType_Bishop + PieceType_Black },
		{ 'n', PieceType_Knight + PieceType_Black },
		{ 'q', PieceType_Queen + PieceType_Black },
		{ 'k', PieceType_King + PieceType_Black },

		{ 'P', PieceType_Pawn + PieceType_White },
		{ 'R', PieceType_Rook + PieceType_White },
		{ 'B', PieceType_Bishop + PieceType_White },
		{ 'N', PieceType_Knight + PieceType_White },
		{ 'Q', PieceType_Queen + PieceType_White },
		{ 'K', PieceType_King + PieceType_White },
	};

	std::vector<Move> Pawn::GetVaildMoves() const
	{
		auto direction = IsBlack() ? glm::vec2(0.f, 1.f) : glm::vec2(0.f, -1.f);
		if (moved)
			return { { direction, 1 } };
		else
			return { { direction, 2 } };
	}

	std::vector<Move> Rook::GetVaildMoves() const
	{
		return
		{
			{ glm::vec2(1.f, 0.f), -1},
			{ glm::vec2(-1.f, 0.f), -1},
			{ glm::vec2(0.f, 1.f), -1},
			{ glm::vec2(0.f, -1.f), -1},
		};
	}

	std::vector<Move> Knight::GetVaildMoves() const
	{
		return
		{
			{ glm::vec2(1.f, 2.f), 1},
			{ glm::vec2(-1.f, 2.f), 1},
			{ glm::vec2(1.f, -2.f), 1},
			{ glm::vec2(-1.f, -2.f), 1},
			{ glm::vec2(2.f, 1.f), 1},
			{ glm::vec2(-2.f, 1.f), 1},
			{ glm::vec2(2.f, -1.f), 1},
			{ glm::vec2(-2.f, -1.f), 1},
		};
	}

	std::vector<Move> Bishop::GetVaildMoves() const
	{
		return
		{
			{ glm::vec2(1.f, 1.f), -1},
			{ glm::vec2(1.f, -1.f), -1},
			{ glm::vec2(-1.f, -1.f), -1},
			{ glm::vec2(-1.f, 1.f), -1},
		};
	}

	std::vector<Move> Queen::GetVaildMoves() const
	{
		return
		{
			{ glm::vec2(1.f, 0.f), -1},
			{ glm::vec2(-1.f, 0.f), -1},
			{ glm::vec2(0.f, 1.f), -1},
			{ glm::vec2(0.f, -1.f), -1},
			{ glm::vec2(1.f, 1.f), -1},
			{ glm::vec2(-1.f, 1.f), -1},
			{ glm::vec2(-1.f, -1.f), -1},
			{ glm::vec2(1.f, -1.f), -1},
		};
	}

	std::vector<Move> King::GetVaildMoves() const
	{
		return
		{
			{ glm::vec2(1.f, 0.f), 1},
			{ glm::vec2(-1.f, 0.f),1},
			{ glm::vec2(0.f, 1.f), 1},
			{ glm::vec2(0.f, -1.f),1},
			{ glm::vec2(1.f, 1.f), 1},
			{ glm::vec2(-1.f, 1.f), 1},
			{ glm::vec2(-1.f, -1.f), 1},
			{ glm::vec2(1.f, -1.f),1},
		};
	}
}

#else

namespace chs
{
	std::unordered_map<char, PieceType> CharToPiece =
	{
		{ 'p', BlackPawn },
		{ 'r', BlackRook },
		{ 'b', BlackBishop },
		{ 'n', BlackKnight },
		{ 'q', BlackQueen },
		{ 'k', BlackKing },

		{ 'P', WhitePawn },
		{ 'R', WhiteRook },
		{ 'B', WhiteBishop },
		{ 'N', WhiteKnight },
		{ 'Q', WhiteQueen },
		{ 'K', WhiteKing },
	};

	std::unordered_map<PieceType, char> PieceToChar =
	{
		{ BlackPawn,   'p' },
		{ BlackRook,   'r' },
		{ BlackBishop, 'b' },
		{ BlackKnight, 'n' },
		{ BlackQueen,  'q' },
		{ BlackKing,   'k' },

		{ WhitePawn,   'P' },
		{ WhiteRook,   'R' },
		{ WhiteBishop, 'B' },
		{ WhiteKnight, 'N' },
		{ WhiteQueen,  'Q' },
		{ WhiteKing,   'K' },
	};
}

#endif