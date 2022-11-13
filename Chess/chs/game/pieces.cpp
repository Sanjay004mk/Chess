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

	extern int32_t scores[13] =
	{
		0,     // empty

		1,	   // pawn
		1,

		5,	  // rook
		5,

		3,	  // knight
		3,

		3,	  // bishop
		3,

		9,	  // queen
		9,

		10,	  // king
		10,
	};

	PieceWeight::PieceWeight(PieceType piece)
	{
		ET_DEBUG_ASSERT(piece < 13);
		if (piece == 0)
			return;

		if (IsPawn(piece))
		{
			weights =
			{
				0	,	0	,	0	,	0	,	0	,	0	,	0	,	0	,
				10	,	10	,	0	,	-10	,	-10	,	0	,	10	,	10	,
				5	,	0	,	0	,	5	,	5	,	0	,	0	,	5	,
				0	,	0	,	10	,	20	,	20	,	10	,	0	,	0	,
				5	,	5	,	5	,	10	,	10	,	5	,	5	,	5	,
				10	,	10	,	10	,	20	,	20	,	10	,	10	,	10	,
				20	,	20	,	20	,	30	,	30	,	20	,	20	,	20	,
				0	,	0	,	0	,	0	,	0	,	0	,	0	,	0
			};
		}
		else if (IsRook(piece))
		{
			weights =
			{
				0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
				0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
				0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
				0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
				0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
				0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
				25	,	25	,	25	,	25	,	25	,	25	,	25	,	25	,
				0	,	0	,	5	,	10	,	10	,	5	,	0	,	0
			};
		}
		else if (IsKnight(piece))
		{
			weights =
			{
				0	,	-10	,	0	,	0	,	0	,	0	,	-10	,	0	,
				0	,	0	,	0	,	5	,	5	,	0	,	0	,	0	,
				0	,	0	,	10	,	10	,	10	,	10	,	0	,	0	,
				0	,	0	,	10	,	20	,	20	,	10	,	5	,	0	,
				5	,	10	,	15	,	20	,	20	,	15	,	10	,	5	,
				5	,	10	,	10	,	20	,	20	,	10	,	10	,	5	,
				0	,	0	,	5	,	10	,	10	,	5	,	0	,	0	,
				0	,	0	,	0	,	0	,	0	,	0	,	0	,	0
			};
		}
		else if (IsBishop(piece))
		{
			weights =
			{
				0	,	0	,	-10	,	0	,	0	,	-10	,	0	,	0	,
				0	,	0	,	0	,	10	,	10	,	0	,	0	,	0	,
				0	,	0	,	10	,	15	,	15	,	10	,	0	,	0	,
				0	,	10	,	15	,	20	,	20	,	15	,	10	,	0	,
				0	,	10	,	15	,	20	,	20	,	15	,	10	,	0	,
				0	,	0	,	10	,	15	,	15	,	10	,	0	,	0	,
				0	,	0	,	0	,	10	,	10	,	0	,	0	,	0	,
				0	,	0	,	0	,	0	,	0	,	0	,	0	,	0
			};
		}

		// vertical flip weights for black pieces;
		if (IsBlack(piece))
		{
			auto temp = weights;
			for (size_t i = 0; i < 8; i++)
				memcpy_s(&weights[i * 8], sizeof(int32_t) * 8, &temp[(7 - i) * 8], sizeof(int32_t) * 8);
		}
	}

	extern PieceWeight positionWeights[13] =
	{
		PieceWeight(0),
		PieceWeight(1),
		PieceWeight(2),
		PieceWeight(3),
		PieceWeight(4),
		PieceWeight(5),
		PieceWeight(6),
		PieceWeight(7),
		PieceWeight(8),
		PieceWeight(9),
		PieceWeight(10),
		PieceWeight(11),
		PieceWeight(12)
	};
}
