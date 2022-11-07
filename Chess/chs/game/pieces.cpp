#include "pieces.h"

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
}