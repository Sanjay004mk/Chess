#pragma once
#include <glm/glm.hpp>
#include <unordered_map>

#include "Entropy/EntropyUtils.h"

#if 0

namespace chs
{
	using PieceType = uint32_t;
	enum : PieceType
	{
		PieceType_Pawn = 0,
		PieceType_Rook = 2,
		PieceType_Knight = 4,
		PieceType_Bishop = 6,
		PieceType_Queen = 8,
		PieceType_King = 10,

		PieceType_Black = 0,
		PieceType_White = 1,
	};

	extern std::unordered_map<char, PieceType> CharToPiece;

	struct Move
	{
		glm::vec2 direction = {};
		int32_t numTiles = 0;
	};

	class Piece
	{
	public:
		Piece(PieceType type) : type(type) {}
		virtual ~Piece() {}

		virtual std::vector<Move> GetVaildMoves() const = 0;

		bool IsBlack() const { return !(type % 2); }
		PieceType GetType() const { return type % 2 == 0 ? type : type - 1; }

		const PieceType type;
		bool moved = false;
	};

	class Pawn : public Piece
	{
	public:
		Pawn(bool isBlack) : Piece(PieceType_Pawn | (isBlack ? PieceType_Black : PieceType_White)) {}
		Pawn(PieceType type) : Piece(type) {}
		~Pawn() {}

		virtual std::vector<Move> GetVaildMoves() const override;
	};

	class Rook : public Piece
	{
	public:
		Rook(bool isBlack) : Piece(PieceType_Rook | (isBlack ? PieceType_Black : PieceType_White)) {}
		Rook(PieceType type) : Piece(type) {}
		~Rook() {}

		virtual std::vector<Move> GetVaildMoves() const override;
	};

	class Knight : public Piece
	{
	public:
		Knight(bool isBlack) : Piece(PieceType_Knight | (isBlack ? PieceType_Black : PieceType_White)) {}
		Knight(PieceType type) : Piece(type) {}
		~Knight() {}

		virtual std::vector<Move> GetVaildMoves() const override;
	};

	class Bishop : public Piece
	{
	public:
		Bishop(bool isBlack) : Piece(PieceType_Bishop | (isBlack ? PieceType_Black : PieceType_White)) {}
		Bishop(PieceType type) : Piece(type) {}
		~Bishop() {}

		virtual std::vector<Move> GetVaildMoves() const override;
	};

	class Queen : public Piece
	{
	public:
		Queen(bool isBlack) : Piece(PieceType_Queen | (isBlack ? PieceType_Black : PieceType_White)) {}
		Queen(PieceType type) : Piece(type) {}
		~Queen() {}

		virtual std::vector<Move> GetVaildMoves() const override;
	};

	class King : public Piece
	{
	public:
		King(bool isBlack) : Piece(PieceType_King | (isBlack ? PieceType_Black : PieceType_White)) {}
		King(PieceType type) : Piece(type) {}
		~King() {}

		virtual std::vector<Move> GetVaildMoves() const override;
	};
}

#else

namespace chs
{
	using PieceType = uint32_t;
	using Color = uint32_t;
	enum : PieceType
	{
		Empty = 0,
		BlackPawn,
		WhitePawn,
		BlackRook,
		WhiteRook,
		BlackKnight,
		WhiteKnight,
		BlackBishop,
		WhiteBishop,
		BlackQueen,
		WhiteQueen,
		BlackKing,
		WhiteKing,
	};

	enum : Color
	{
		Black = 0,
		White,
		Both
	};

	enum : uint32_t
	{
		CastleBlackKing = 1,
		CastleBlackQueen = 2,
		CastleWhiteKing = 4,
		CastleWhiteQueen = 8,
	};

	struct PieceList
	{
		// number of pieces of 'this' type on the board
		uint32_t count = 0;
		// index ( 0 - 63 ) of count pieces indicating their position on the board. 
		// 10 pieces of a type can exist at most ( 8 converted pawns + 2 default )
		std::array<int32_t, 10> positions = { -1 };
	};

	extern std::unordered_map<char, PieceType> CharToPiece;
	extern std::unordered_map<PieceType, char> PieceToChar;
	
	extern bool IsMajor[13];
	extern bool IsMinor[13];

	inline Color GetColor(PieceType piece)
	{
		return (piece % 2 == 0) ? White : Black;
	}

	inline bool IsWhite(PieceType piece)
	{
		return GetColor(piece);
	}

	inline bool IsBlack(PieceType piece)
	{
		return !GetColor(piece);
	}

	inline bool IsPawn(PieceType piece)
	{
		return piece > 0 && piece < 3;
	}

	inline bool IsRook(PieceType piece)
	{
		return piece > 2 && piece < 5;
	}

	inline bool IsKnight(PieceType piece)
	{
		return piece > 4 && piece < 7;
	}

	inline bool IsBishop(PieceType piece)
	{
		return piece > 6 && piece < 9;
	}

	inline bool IsQueen(PieceType piece)
	{
		return piece > 8 && piece < 11;
	}

	inline bool IsKing(PieceType piece)
	{
		return piece > 10 && piece < 13;
	}

	inline bool CanCastle(int32_t castlePerm, Color color, bool QueenSide)
	{
		int32_t mask = 1;
		// white king
		if (color)
			mask = mask << 2;

		if (QueenSide)
			mask = mask << 1;

		return castlePerm & mask;
	}

	template <typename T = float>
	inline glm::vec<2, T, glm::defaultp> GetPositionFromIndex(int32_t index)
	{
		int32_t x = (index % 8) + 1;
		int32_t y = (index / 8) + 1;
		ET_DEBUG_ASSERT(x < 9 && x > 0);
		ET_DEBUG_ASSERT(y < 9 && y > 0);
		return { static_cast<T>(x), static_cast<T>(y) };
	}

	template <typename T = float>
	inline int32_t GetIndexFromPosition(const glm::vec<2, T, glm::defaultp>& position)
	{
		int32_t index = static_cast<int32_t>(position.x - static_cast<T>(1));
		index += (8 * static_cast<int32_t>(position.y - static_cast<T>(1)));
		ET_DEBUG_ASSERT(index < 64);
		return index;
	}

	inline int32_t FileRankToIndex(char file, char rank)
	{
		int32_t index = static_cast<int32_t>((file - 'a'));
		index += static_cast<int32_t>(8 * (rank - '1'));
		return index;

	}

	inline std::pair<char, char> IndexToFileRank(int32_t index)
	{
		int32_t x = (index % 8);
		int32_t y = (index / 8);
		return { 'a' + x, '1' + y };
	}

	inline std::string IndexToStr(int32_t index)
	{
		char str[3] = {0};
		auto [file, rank] = IndexToFileRank(index);
		str[0] = file;
		str[1] = rank;
		return { str };
	}
}

#endif