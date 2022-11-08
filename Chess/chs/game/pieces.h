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
	using PieceType = uint8_t;
	using Color = uint8_t;
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
	};

	enum : uint8_t
	{
		CastleBlackKing = 1,
		CastleBlackQueen = 2,
		CastleWhiteKing = 4,
		CastleWhiteQueen = 8,
	};

	struct PieceList
	{
		// number of pieces of 'this' type on the board
		uint8_t count = 0;
		// index ( 0 - 63 ) of count pieces indicating their position on the board. 
		// 10 pieces of a type can exist at most ( 8 converted pawns + 2 default )
		uint8_t positions[10] = { 0 };
	};

	extern std::unordered_map<char, PieceType> CharToPiece;
	extern std::unordered_map<PieceType, char> PieceToChar;

	inline Color GetColor(PieceType piece)
	{
		return (piece % 2 == 0) ? White : Black;
	}

	inline bool CanCastle(uint8_t castlePerm, Color color, bool QueenSide)
	{
		uint8_t mask = 1;
		// white king
		if (color)
			mask = mask << 2;

		if (QueenSide)
			mask = mask << 1;

		return castlePerm & mask;
	}

	template <typename T>
	inline glm::vec<2, T, glm::defaultp> GetPositionFromIndex(uint8_t index)
	{
		int32_t x = (index % 8) + 1;
		int32_t y = (index / 8) + 1;
		ET_DEBUG_ASSERT(x < 9 && x > 0);
		ET_DEBUG_ASSERT(y < 9 && y > 0);
		return { static_cast<T>(x), static_cast<T>(y) };
	}

	template <typename T>
	inline uint8_t GetIndexFromPosition(const glm::vec<2, T, glm::defaultp>& position)
	{
		uint8_t index = static_cast<uint8_t>(position.x - static_cast<T>(1));
		index += static_cast<uint8_t>(8 * static_cast<int32_t>(position.y - static_cast<T>(1)));
		ET_DEBUG_ASSERT(index < 64);
		return index;
	}

	inline uint8_t FileRankToIndex(char file, char rank)
	{
		uint8_t index = static_cast<uint8_t>((file - 'a'));
		index += static_cast<uint8_t>(8 * (rank - '1'));
		return index;

	}

	inline std::pair<char, char> IndexToFileRank(uint8_t index)
	{
		int32_t x = (index % 8);
		int32_t y = (index / 8);
		return { 'a' + x, '1' + y };
	}
}

#endif