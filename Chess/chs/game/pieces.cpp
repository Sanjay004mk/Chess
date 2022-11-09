#include "pieces.h"

namespace chs
{
	std::unordered_map<char, PieceType> CharToPiece =
	{
		{ 'p', BlackPawn   },
		{ 'r', BlackRook   },
		{ 'b', BlackBishop },
		{ 'n', BlackKnight },
		{ 'q', BlackQueen  },
		{ 'k', BlackKing   },

		{ 'P', WhitePawn   },
		{ 'R', WhiteRook   },
		{ 'B', WhiteBishop },
		{ 'N', WhiteKnight },
		{ 'Q', WhiteQueen  },
		{ 'K', WhiteKing   },
	};

	std::unordered_map<PieceType, char> PieceToChar =
	{
		{ Empty,       '-' },

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

	bool IsMajor[13] =
	{
		false,    // empty

		false,    // pawn
		false,

		true,    // rook
		true,

		false,   // knight
		false,

		false,   // bishop
		false,

		true,    // queen
		true, 

		false,   // king
		false,

	};
	
	bool IsMinor[13] =
	{
		false,     // empty

		false,	   // pawn
		false,

		false,	  // rook
		false,

		true,	  // knight
		true,

		true,	  // bishop
		true,

		false,	  // queen
		false,

		false,	  // king
		false,
	};
}
