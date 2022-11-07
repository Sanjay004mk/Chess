#pragma once
#include <glm/glm.hpp>
#include <unordered_map>

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

	};

	class Piece
	{
	public:
		Piece(PieceType type) : type(type), position({}) {}
		virtual ~Piece() {}

		virtual std::vector<Move> GetVaildMoves() const = 0;

		bool IsBlack() const { return !(type % 2); }
		PieceType GetType() const { return type % 2 == 0 ? type : type - 1; }

		const PieceType type;
		glm::vec2 position;
	};

	class Pawn : public Piece
	{
	public:
		Pawn(bool isBlack) : Piece(PieceType_Pawn | (isBlack ? PieceType_Black : PieceType_White)) {}
		Pawn(PieceType type) : Piece(type) {}
		~Pawn() {}

		virtual std::vector<Move> GetVaildMoves() const override { return {}; }
	};

	class Rook : public Piece
	{
	public:
		Rook(bool isBlack) : Piece(PieceType_Rook | (isBlack ? PieceType_Black : PieceType_White)) {}
		Rook(PieceType type) : Piece(type) {}
		~Rook() {}

		virtual std::vector<Move> GetVaildMoves() const override { return {}; }
	};

	class Knight : public Piece
	{
	public:
		Knight(bool isBlack) : Piece(PieceType_Knight | (isBlack ? PieceType_Black : PieceType_White)) {}
		Knight(PieceType type) : Piece(type) {}
		~Knight() {}

		virtual std::vector<Move> GetVaildMoves() const override { return {}; }
	};

	class Bishop : public Piece
	{
	public:
		Bishop(bool isBlack) : Piece(PieceType_Bishop | (isBlack ? PieceType_Black : PieceType_White)) {}
		Bishop(PieceType type) : Piece(type) {}
		~Bishop() {}

		virtual std::vector<Move> GetVaildMoves() const override { return {}; }
	};

	class Queen : public Piece
	{
	public:
		Queen(bool isBlack) : Piece(PieceType_Queen | (isBlack ? PieceType_Black : PieceType_White)) {}
		Queen(PieceType type) : Piece(type) {}
		~Queen() {}

		virtual std::vector<Move> GetVaildMoves() const override { return {}; }
	};

	class King : public Piece
	{
	public:
		King(bool isBlack) : Piece(PieceType_King | (isBlack ? PieceType_Black : PieceType_White)) {}
		King(PieceType type) : Piece(type) {}
		~King() {}

		virtual std::vector<Move> GetVaildMoves() const override { return {}; }
	};
}