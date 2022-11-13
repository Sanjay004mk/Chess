#pragma once
#include <glm/gtx/hash.hpp>

#include "pieces.h"

#if defined(CHS_DEBUG)
#define CHS_MOVE_STORE_EXTRA_INFO
#endif

#include "move.h"

#define MAX_DEPTH 64
// used for memory allocation while generating moves
#define MAX_MOVES 256

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
		PieceIterator operator++(int) { auto ret = *this; Inc(); return ret; }
		PieceIterator& operator--() { Dec(); return *this; }
		PieceIterator operator--(int) { auto ret = *this; Dec(); return ret; }
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

	using GetMoveFn = void(*)(const Board*, int32_t, std::vector<Move>&);

	// used for vs Player / vs Computer
	struct BoardSpecification
	{
		Color player[2] = { White, Black };
	};

	class Board
	{
	public:
		Board(std::string_view fen_string, const BoardSpecification& specs = {});
		~Board() {}

		std::unordered_map<glm::vec2, Move> GetMoveTiles(const glm::vec2& position);
		bool MakeMove(Move move, bool& shouldPromote);
		bool Promote(PieceType piece);
		bool Undo();
		bool Valid() const;

		PieceType GetTile(const glm::vec2& tile);
		PieceType GetTile(int32_t index);
		bool IsPiece(const glm::vec2& tile);
		bool IsPiece(int32_t index);

		bool IsAttacked(int32_t index, Color by) const;
		bool IsRepeated() const { return std::find(poskeys.begin(), poskeys.end(), hashKey) != poskeys.end(); }
		std::pair<bool, Color> InCheckMate() const { return { checkmate, turn }; }

		PieceIterator begin() { PieceIterator it(0, tiles); if (!it) ++it; return it; }
		PieceIterator end() { return PieceIterator(64, tiles); }

		const auto& CapturedPieces() const { return capturedTiles; }
		auto Full() const { return fullMoves; }
		auto Fifty() const { return fiftyMove; }
		auto GetTurn() const { return turn; }

		size_t hash() const;
		size_t GetHash() const { return hashKey; }

		int32_t GetMaterialScore(Color side) const { ET_DEBUG_ASSERT(side < 2); return materialScore[side]; }
		int32_t Evaluate(Color side) const;

		Move Search(int32_t depth);

		std::string GetFEN() const;

		size_t PerftTest(int32_t& capture, int32_t& ep, int32_t& castles, int32_t& prom,
			std::vector<std::vector<Move>>& depth_moves, int32_t depth = 6);
		void PerftRoot(int32_t depth);

	private:
		std::vector<Move> GetAllMoves(Color side);
		void GetAllMoves(std::vector<Move>& moves, Color side);
		void ResetForSearch();
		int32_t AlphaBeta(int32_t alpha, int32_t beta, int32_t depth);

		int32_t CalcMaterialScore(Color side) const;

		bool IsInCheck(Color side);
		bool AddPiece(int32_t index, PieceType piece);
		bool RemovePiece(int32_t index, PieceType* piece = nullptr);
		bool ShiftPiece(int32_t from, int32_t to, PieceType* piece = nullptr);
		bool MovePiece(Move& move);
		bool Revert();
		void LoadFromFen(std::string_view fen_string);
		std::vector<Move> GetMoveTiles(int32_t position);

		BoardSpecification specs;
		PieceType tiles[64] = { 0 };		
		std::vector<std::pair<int32_t, PieceType>> capturedTiles;
		std::vector<std::pair<Move, MoveMetaData>> playedMoves;
		std::vector<size_t> poskeys;

		// to be able to index from PieceType enum ( 0 = empty, ..., 12 = white king )
		PieceList pieces[13] = {};

		Color player = White;
		Color turn = 0;
		int32_t enPassant = 64;
		int32_t castlePermission = 0;
		int32_t fiftyMove = 0;
		int32_t fullMoves = 0;

		bool inCheck = false;
		bool checkmate = false;

		int32_t materialScore[2];

		size_t hashKey = 0;

		// bit boards 
		uint64_t pawnBitBoard[2] = { 0 };
		uint64_t majorBitBoard[2] = { 0 };
		uint64_t minorBitBoard[2] = { 0 };

		// for move searching
		std::array<Move, MAX_DEPTH> pv_moves;

		template <typename ostream>
		friend ostream& operator<<(ostream& stream, const Board& board);

		friend struct PieceIterator;

		friend void Slide(const Board* board, int32_t index, const glm::ivec2& direction, std::vector<Move>& moves);
		friend void PawnMoves(const Board* board, int32_t index, std::vector<Move>& moves);
		friend void RookMoves(const Board* board, int32_t index, std::vector<Move>& moves);
		friend void KnightMoves(const Board* board, int32_t index, std::vector<Move>& moves);
		friend void BishopMoves(const Board* board, int32_t index, std::vector<Move>& moves);
		friend void QueenMoves(const Board* board, int32_t index, std::vector<Move>& moves);
		friend void KingMoves(const Board* board, int32_t index, std::vector<Move>& moves);
	};

	void PreComputeBoardHashes();

	inline void SetBit(uint64_t& bit_board, int32_t bit, int32_t position)
	{
		uint64_t mask = (1ull) << position;
		bit_board = (bit_board & ~mask);
		bit_board |= (uint64_t)bit << position;
	}

	inline int32_t GetBit(uint64_t bit_board, int32_t position)
	{
		return (bit_board & (1ull << position)) >> position;
	}

	inline int32_t GetNumBits(uint64_t bit_board)
	{
		int32_t num = 0;
		while (bit_board)
		{
			if (bit_board & 1ull)
				num++;
			
			bit_board = (bit_board >> 1);
		}
		return num;
	}

	inline bool InsideBoard(const glm::vec2& pos)
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
}