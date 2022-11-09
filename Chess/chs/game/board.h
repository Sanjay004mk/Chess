#pragma once
#include <glm/gtx/hash.hpp>

#include "pieces.h"

namespace chs
{
	class Board;

	struct PieceRenderInfo
	{
		int32_t type = 0;
		glm::vec2 position = {};
	};

	struct PieceIterator
	{
		PieceIterator(uint8_t index, PieceType* tiles)
			: index(index), tiles(tiles)
		{}
		PieceIterator(const PieceIterator&) = default;

		PieceRenderInfo operator*() { return Deref(); }
		const PieceRenderInfo operator*() const { return Deref(); }
		PieceIterator& operator++() { Inc(); return *this; }
		PieceIterator& operator++(int) { auto ret = *this; Inc(); return ret; }
		PieceIterator& operator--() { Dec(); return *this; }
		PieceIterator& operator--(int) { auto ret = *this; Dec(); return ret; }
		operator bool() const { return Valid(); }
		bool operator==(const PieceIterator& other) const { return index == other.index; }
		bool operator!=(const PieceIterator& other) const {	return !(*this == other); }
		bool operator>(const PieceIterator& other) const { return index > other.index; }
		bool operator<(const PieceIterator& other) const { return index < other.index; }
		bool operator>=(const PieceIterator& other) const {	return !(*this < other); }
		bool operator<=(const PieceIterator& other) const {	return !(*this > other); }

	private:
		void Inc();
		void Dec();
		PieceRenderInfo Deref() const;
		bool Valid() const;

		PieceType* tiles = nullptr;
		int32_t index;

		friend class Board;
	};

	struct Move
	{
		Move() {}
		Move(int32_t from, int32_t to, int32_t capture, int32_t pawnStart = 0, int32_t enPassant = 0, int32_t castle = 0, PieceType promote = 0)
		{
			From(from);
			To(to);
			EnPassant(enPassant);
			PawnStart(pawnStart);
			Castle(castle);
			Capture(capture);
			PromotedTo(promote);
		}

		template <typename integer>
		Move(integer data) : data(static_cast<uint32_t>(data)) {}

		bool Valid() const
		{
			return true;
		}

		int32_t From() const { return (int32_t)(data & 0x0000003fu); }
		void From(int32_t index)
		{
			// clear from
			data = (data & ~0x0000003fu);
			data |= (index & 0x0000003fu);
		}

		int32_t To() const { return (int32_t)((data & 0x00000fc0u) >> 6); }
		void To(int32_t index)
		{
			// clear to
			data = (data & ~0x00000fc0u);
			data |= ((index & 0x0000003fu) << 6);
		}

		bool EnPassant() const { return data & 0x00001000; }
		void EnPassant(bool isEnPassant)
		{
			// clear en passant
			data = (data & ~0x00001000u);
			data |= ((isEnPassant & 1u) << 12);
		}

		bool PawnStart() const { return data & 0x00002000; }
		void PawnStart(bool isPawnStart)
		{
			// clear pawn start
			data = (data & ~0x00002000u);
			data |= ((isPawnStart & 1u) << 13);
		}

		bool Castle() const { return data & 0x00004000u; }
		void Castle(bool isCastle)
		{
			// clear castle
			data = (data & ~0x00004000u);
			data |= ((isCastle & 1u) << 14);
		}

		bool Capture() const { return data & 0x00008000; }
		void Capture(bool isCapture)
		{
			// clear capture
			data = (data & ~0x00008000u);
			data |= ((isCapture & 1u) << 15);
		}

		PieceType PromotedTo() const { return (PieceType)((data & 0x000f0000) >> 16); }
		void PromotedTo(PieceType type)
		{
			// clear promoted to
			data = (data & ~0x000f0000u);
			data |= ((uint32_t)type << 16);
		}

		PieceType Type() const { return (PieceType)((data & 0x00f00000) >> 20); }
		void Type(PieceType type)
		{
			// clear type
			data = (data & ~0x00f00000u);
			data |= ((uint32_t)type << 20);
		}

		uint32_t data = 0;
		/*
		*    
		*   0000  0000    0000  0000    0000  0000    0000  0000
		*   ----  ----    ----  ----    ----  ----    --11  1111  -> 'from'        Mask: 0x0000003f    Values: ( 0 - 63 ) 
		*   ----  ----    ----  ----    ----  1111    11--  ----  -> 'to'	       Mask: 0x00000fc0    Values: ( 0 - 63 )
		*   ----  ----    ----  ----    ---1  ----    ----  ----  -> 'en passsant' Mask: 0x00001000    Values: 0 / 1
		*   ----  ----    ----  ----    --1-  ----    ----  ----  -> 'pawn start'  Mask: 0x00002000    Values: 0 / 1  ( set only on 2 long moves )
		*   ----  ----    ----  ----    -1--  ----    ----  ----  -> 'castle'      Mask: 0x00004000    Vaules: 0 / 1
		*   ----  ----    ----  ----    1---  ----    ----  ----  -> 'capture'     Mask: 0x00008000    Vaules: 0 / 1
		*   ----  ----    ----  1111    ----  ----    ----  ----  -> 'promote'     Mask: 0x000f0000    Vaules: ( 0 - 12 )
		*   ----  ----    1111  ----    ----  ----    ----  ----  -> 'type'        Mask: 0x00f00000    Vaules: ( 1 - 12 )  [[maybe unused]]
		*/

		template <typename ostream>
		friend ostream& operator<<(ostream& stream, const Move& move);
	};

	using GetMoveFn = std::vector<Move>(*)(const Board*, int32_t);

	extern GetMoveFn GetMoves[13];

	struct MoveList
	{
		MoveList()
		{
			moves.reserve(64);
		}
		
		std::vector<Move> moves;
	};

	class Board
	{
	public:
		Board(std::string_view fen_string);
		~Board() {}

		std::unordered_map<glm::vec2, Move> GetMoveTiles(const glm::vec2& position);
		bool MovePiece(Move move);
		bool Valid();

		PieceType GetTile(const glm::vec2& tile);
		PieceType GetTile(int32_t index);
		bool IsPiece(const glm::vec2& tile);
		bool IsPiece(int32_t index);

		bool IsAttacked(int32_t index, Color by) const;

		PieceIterator begin() { PieceIterator it(0, tiles); if (!it) ++it; return it; }
		PieceIterator end() { return PieceIterator(64, tiles); }

		const auto& CapturedPieces() const { return capturedTiles; }

		size_t hash() const;

	private:
		bool AddPiece(int32_t index, PieceType piece);
		bool RemovePiece(int32_t index, PieceType* piece = nullptr);
		bool ShiftPiece(int32_t from, int32_t to, PieceType* piece = nullptr);
		void LoadFromFen(std::string_view fen_string);
		std::vector<Move> GetMoveTiles(int32_t position);

		PieceType tiles[64] = { 0 };		
		std::vector<std::pair<int32_t, PieceType>> capturedTiles;
		// to be able to index from PieceType enum ( 0 = empty, ..., 12 = white king )
		PieceList pieces[13] = {};

		Color turn = 0;
		int32_t enPassant = 64;
		int32_t castlePermission = 0;
		int32_t fiftyMove = 0;
		int32_t fullMoves = 0;

		size_t hashKey = 0;

		// bit boards 
		uint64_t pawnBitBoard[2];
		uint64_t majorBitBoard[2];
		uint64_t minorBitBoard[2];

		template <typename ostream>
		friend ostream& operator<<(ostream& stream, const Board& board);

		friend struct PieceIterator;

		friend std::vector<Move> Slide(const Board* board, int32_t index, const glm::ivec2& direction);
		friend std::vector<Move> PawnMoves(const Board* board, int32_t index);
		friend std::vector<Move> RookMoves(const Board* board, int32_t index);
		friend std::vector<Move> KnightMoves(const Board* board, int32_t index);
		friend std::vector<Move> BishopMoves(const Board* board, int32_t index);
		friend std::vector<Move> QueenMoves(const Board* board, int32_t index);
		friend std::vector<Move> KingMoves(const Board* board, int32_t index);
	};

	inline void SetBit(uint64_t& bit_board, int32_t bit, int32_t position)
	{
		uint64_t mask = (1ull) << position;
		bit_board = (bit_board & ~mask);
		bit_board |= (uint64_t)bit << position;
	}

	inline int32_t GetBit(uint64_t bit_board, int32_t position)
	{
		return bit_board & (1ull << position);
	}

	inline 	bool InsideBoard(const glm::vec2& pos)
	{
		return pos.x > 0.f && pos.x < 9.f &&
			pos.y > 0.f && pos.y < 9.f;
	}

	inline int32_t EnPassantToPiece(int32_t enp)
	{
		return (enp < 24) ? enp + 8 : enp - 8;
	}

	inline int32_t PieceToEnPassant(int32_t piece)
	{
		return (piece < 32) ? piece - 8 : piece + 8;
	}

	template <typename ostream>
	ostream& operator<<(ostream& stream, const Move& move)
	{
		stream << "Move: [ " << "from: " << IndexToStr(move.From()) << ", to: " << IndexToStr(move.To()) << ", en passant: " << move.EnPassant();
		stream << ", pawn start: " << move.PawnStart() << ", castle: " << move.Castle() << ", capture: " << move.Capture();
		stream << ", promoted to: " << PieceToChar[move.PromotedTo()] << ", type: " << PieceToChar[move.Type()] << " ]";
		return stream;
 	}

	template <typename ostream>
	ostream& operator<<(ostream& stream, const Board& board)
	{
		stream << std::endl << "board" << std::endl;
		for (int32_t i = 63; i > -1; i--)
		{
			if (i % 8 == 7)
			{
				stream << std::endl;
				stream << (i / 8) + 1 << "   ";
			}
			// horizontal flip
			int32_t index = (i - (i % 8)) + (7 - (i % 8));
			if (board.tiles[index])
				stream << PieceToChar[board.tiles[index]];
			else
				stream << "*";

			stream << "  ";
		}
		stream << std::endl << std::endl;
		stream << "    " << "a  b  c  d  e  f  g  h" << std::endl << std::endl;
		stream << "turn:";
		if (board.turn)
			stream << 'w';
		else
			stream << 'b';
		stream << std::endl;
		
		stream << "en passant: ";
		if (board.enPassant < 64)
		{
			auto [file, rank] = IndexToFileRank(board.enPassant);
			stream << file << rank;
		}
		else
			stream << '-';
		stream << std::endl;

		stream << "castling: ";
		for (int32_t i = 0; i < 4; i++)
		{
			if (board.castlePermission & (1 << i))
			{
				switch (i)
				{
				case 0: stream << 'k'; break;
				case 1: stream << 'q'; break;
				case 2: stream << 'K'; break;
				case 3: stream << 'Q'; break;
				}
			}
		}
		stream << std::endl;
		stream << "fifty moves: " << std::hex << board.fiftyMove << std::endl;
		stream << "full moves: " << std::hex << board.fullMoves << std::endl;
		stream << "hash: 0x" << std::hex << board.hashKey << std::endl;

		return stream;
	}
}

namespace std
{
	template <>
	struct hash<chs::Move>
	{
		size_t operator()(const chs::Move& move) const
		{
			return (size_t)move.data;
		}
	};
}