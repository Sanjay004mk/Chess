#pragma once
#include <glm/gtx/hash.hpp>

#include "pieces.h"

#if defined(CHS_DEBUG)
#define CHS_MOVE_STORE_EXTRA_INFO
#endif

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

		bool Valid() const;

		int32_t From() const { return (int32_t)(data & 0x0000003fu); }
		void From(int32_t index)
		{
			// clear from
			data = (data & ~0x0000003fu);
			data |= (index & 0x0000003fu);
#if defined(CHS_MOVE_STORE_EXTRA_INFO)
			auto [file, rank] = IndexToFileRank(index);
			from[0] = file;
			from[1] = rank;
#endif
		}

		int32_t To() const { return (int32_t)((data & 0x00000fc0u) >> 6); }
		void To(int32_t index)
		{
			// clear to
			data = (data & ~0x00000fc0u);
			data |= ((index & 0x0000003fu) << 6);

#if defined(CHS_MOVE_STORE_EXTRA_INFO)
			auto [file, rank] = IndexToFileRank(index);
			to[0] = file;
			to[1] = rank;
#endif
		}

		bool EnPassant() const { return data & 0x00001000; }
		void EnPassant(bool isEnPassant)
		{
			// clear en passant
			data = (data & ~0x00001000u);
			data |= ((isEnPassant & 1u) << 12);

#if defined(CHS_MOVE_STORE_EXTRA_INFO)
			enp = isEnPassant;
#endif
		}

		bool PawnStart() const { return data & 0x00002000; }
		void PawnStart(bool isPawnStart)
		{
			// clear pawn start
			data = (data & ~0x00002000u);
			data |= ((isPawnStart & 1u) << 13);

#if defined(CHS_MOVE_STORE_EXTRA_INFO)
			pawnStart = isPawnStart;
#endif
		}

		bool Castle() const { return data & 0x00004000u; }
		void Castle(bool isCastle)
		{
			// clear castle
			data = (data & ~0x00004000u);
			data |= ((isCastle & 1u) << 14);

#if defined(CHS_MOVE_STORE_EXTRA_INFO)
			castle = isCastle;
#endif
		}

		bool Capture() const { return data & 0x00008000; }
		void Capture(bool isCapture)
		{
			// clear capture
			data = (data & ~0x00008000u);
			data |= ((isCapture & 1u) << 15);

#if defined(CHS_MOVE_STORE_EXTRA_INFO)
			capture = isCapture;
#endif
		}

		bool IsPromoted() const { return (data2 & 0x08000000u); }
		void IsPromoted(bool isPromoted)
		{
			// clear is promoted
			data2 = (data2 & ~0x08000000u);
			data2 |= ((isPromoted & 1u) << 27);

#if defined(CHS_MOVE_STORE_EXTRA_INFO)
			promoted = isPromoted;
#endif
		}

		PieceType PromotedTo() const { return (PieceType)((data & 0x000f0000) >> 16); }
		void PromotedTo(PieceType type)
		{
			// clear promoted to
			data = (data & ~0x000f0000u);
			data |= ((uint32_t)type << 16);
			IsPromoted(type);

#if defined(CHS_MOVE_STORE_EXTRA_INFO)
			promotedTo = type;
#endif
		}

		PieceType CapturedType() const { return (PieceType)((data & 0x00f00000) >> 20); }
		void CapturedType(PieceType type)
		{
			// clear type
			data = (data & ~0x00f00000u);
			data |= ((uint32_t)type << 20);

#if defined(CHS_MOVE_STORE_EXTRA_INFO)
			capturedType = type;
#endif
		}

		int32_t Fifty() const { return (int32_t)((data & 0xff000000u) >> 24); }
		void Fifty(int32_t fifty)
		{
			// clear fifty
			data = (data & ~0xff000000u);
			data |= ((uint32_t)fifty << 24);

#if defined(CHS_MOVE_STORE_EXTRA_INFO)
			this->fifty = fifty;
#endif
		}

		int32_t EnPassantTile() const { return (int32_t)(data2 & 0x0000007fu); }
		void EnPassantTile(int32_t index)
		{
			// clear enp
			data2 = (data2 & ~0x0000007fu);
			data2 |= ((uint32_t)index);
#if defined(CHS_MOVE_STORE_EXTRA_INFO)
			auto [file, rank] = IndexToFileRank(index);
			enpassant[0] = file;
			enpassant[1] = rank;
#endif
		}

		int32_t CastlePerm() const { return (int32_t)((data2 & 0x00000780u) >> 7); }
		void CastlePerm(int32_t perm)
		{
			// clear perm
			data2 = (data2 & ~0x00000780u);
			data2 |= ((uint32_t)perm << 7);

#if defined(CHS_MOVE_STORE_EXTRA_INFO)
			castlePerms[0] = perm & CastleBlackKing;
			castlePerms[1] = perm & CastleBlackQueen;
			castlePerms[2] = perm & CastleWhiteKing;
			castlePerms[3] = perm & CastleWhiteQueen;
#endif
		}

		int32_t Full() const { return (int32_t)((data2 & 0x07fff800u) >> 11); }
		void Full(int32_t full)
		{
			// clear perm
			data2 = (data2 & ~0x07fff800u);
			data2 |= ((uint32_t)full << 11);

#if defined(CHS_MOVE_STORE_EXTRA_INFO)
			this->full = full;
#endif
		}

		bool operator==(const Move& other) const
		{
			return data == other.data;
		}

		uint32_t data = 0;
		uint32_t data2 = 0;
		/*
		*    
		*   0000  0000    0000  0000    0000  0000    0000  0000
		*   ----  ----    ----  ----    ----  ----    --11  1111  -> 'from'             Mask: 0x0000003f    Values: ( 0 - 63 ) 
		*   ----  ----    ----  ----    ----  1111    11--  ----  -> 'to'	            Mask: 0x00000fc0    Values: ( 0 - 63 )
		*   ----  ----    ----  ----    ---1  ----    ----  ----  -> 'en passsant'      Mask: 0x00001000    Values: 0 / 1
		*   ----  ----    ----  ----    --1-  ----    ----  ----  -> 'pawn start'       Mask: 0x00002000    Values: 0 / 1  ( set only on 2 long moves )
		*   ----  ----    ----  ----    -1--  ----    ----  ----  -> 'castle'           Mask: 0x00004000    Vaules: 0 / 1
		*   ----  ----    ----  ----    1---  ----    ----  ----  -> 'capture'          Mask: 0x00008000    Vaules: 0 / 1
		*   ----  ----    ----  1111    ----  ----    ----  ----  -> 'promoted to'          Mask: 0x000f0000    Vaules: ( 0 - 12 )
		*   ----  ----    1111  ----    ----  ----    ----  ----  -> 'captured type'    Mask: 0x00f00000    Vaules: ( 0 - 12 )  [used by undo]
		*   1111  1111    ----  ----    ----  ----    ----  ----  -> 'fifty move'       Mask: 0xff000000    Vaules: ( 0 - 100 )  [used by undo]
		* 
		*   next 32 bits   
		* 
		*   0000  0000    0000  0000    0000  0000    0000  0000
		*   ----  ----    ----  ----    ----  ----    -111  1111  -> 'en passant tile'  Mask: 0x0000007f    Values: ( 0 - 64 ) 
		*   ----  ----    ----  ----    ----  -111    1---  ----  -> 'castle perm'      Mask: 0x00000780    Values: ( 0 - 15 )
		*   ----  -111    1111  1111    1111  1---    ----  ----  -> 'full moves'       Mask: 0x07fff800    Values: ( 1 - 65535 )
		*   ----  1---    ----  ----    ----  ----    ----  ----  -> 'is promote'       Mask: 0x08000000    Values: ( 1 - 65535 )
		*/

#if defined(CHS_MOVE_STORE_EXTRA_INFO)
		char from[2] = { 0 };
		char to[2] = { 0 };
		char enpassant[2] = { 0 };
		bool pawnStart = false;
		bool castle = false;
		bool enp = false;
		bool castlePerms[4] = {};
		bool capture = false;
		bool promoted = false;
		PieceType promotedTo = 0;
		PieceType capturedType = 0;
		int32_t fifty = 0;
		int32_t full = 0;
#endif

		template <typename ostream>
		friend ostream& operator<<(ostream& stream, const Move& move);
	};

	using GetMoveFn = void(*)(const Board*, int32_t, std::vector<Move>&);

	class Board
	{
	public:
		Board(std::string_view fen_string);
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

		PieceIterator begin() { PieceIterator it(0, tiles); if (!it) ++it; return it; }
		PieceIterator end() { return PieceIterator(64, tiles); }

		const auto& CapturedPieces() const { return capturedTiles; }
		auto Full() const { return fullMoves; }
		auto Fifty() const { return fiftyMove; }
		auto GetTurn() const { return turn; }

		size_t hash() const;
		size_t GetHash() const { return hashKey; }

		std::string GetFEN() const;

		size_t PerftTest(int32_t& capture, int32_t& ep, int32_t& castles, int32_t& prom, std::vector<std::vector<Move>>& depth_moves, int32_t depth = 6);
		void PerftRoot(int32_t depth);

	private:
		std::vector<Move> GetAllMoves(Color side);
		void GetAllMoves(std::vector<Move>& moves, Color side);
		int32_t GetScore(Color side) const;

		bool IsInCheck(Color side, std::vector<Move>& validMoves);
		bool AddPiece(int32_t index, PieceType piece);
		bool RemovePiece(int32_t index, PieceType* piece = nullptr);
		bool ShiftPiece(int32_t from, int32_t to, PieceType* piece = nullptr);
		bool MovePiece(Move& move);
		bool Revert(Move move);
		void LoadFromFen(std::string_view fen_string);
		std::vector<Move> GetMoveTiles(int32_t position);

		PieceType tiles[64] = { 0 };		
		std::vector<std::pair<int32_t, PieceType>> capturedTiles;
		std::vector<Move> playedMoves;
		std::vector<size_t> poskeys;

		bool inCheck = false;
		std::vector<Move> checkValidMoves;
		// to be able to index from PieceType enum ( 0 = empty, ..., 12 = white king )
		PieceList pieces[13] = {};

		Color player = White;
		Color turn = 0;
		int32_t enPassant = 64;
		int32_t castlePermission = 0;
		int32_t fiftyMove = 0;
		int32_t fullMoves = 0;

		size_t hashKey = 0;

		// bit boards 
		uint64_t pawnBitBoard[2] = { 0 };
		uint64_t majorBitBoard[2] = { 0 };
		uint64_t minorBitBoard[2] = { 0 };

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
		stream << ", castle: " << move.Castle() << ", capture: " << move.Capture();
		move.Valid() ? stream << " (Valid)" : stream << " (Invalid)";
		stream << " ]";
		return stream;
 	}

	template <typename ostream>
	ostream& operator<<(ostream& stream, const Board& board)
	{
		stream << std::endl << "board";
		board.Valid() ? stream << " (Valid)" : stream << " (Invalid)";
		stream << std::endl;
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
		stream << "fen: " << board.GetFEN() << std::endl;

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