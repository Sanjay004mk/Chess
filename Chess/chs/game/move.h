#pragma once
#include <iostream>

#include "pieces.h"

namespace chs
{
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

		bool IsPromoted() const { return (data & 0x00100000u); }
		void IsPromoted(bool isPromoted)
		{
			// clear is promoted
			data = (data & ~0x00100000u);
			data |= ((isPromoted & 1u) << 20);

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

		bool operator==(const Move& other) const { return data == other.data; }
		bool operator!=(const Move& other) const { return data != other.data; }

		uint32_t data = 0;
		/*
		*
		*   0000  0000    0000  0000    0000  0000    0000  0000
		*   ----  ----    ----  ----    ----  ----    --11  1111  -> 'from'             Mask: 0x0000003f    Values: ( 0 - 63 )
		*   ----  ----    ----  ----    ----  1111    11--  ----  -> 'to'	            Mask: 0x00000fc0    Values: ( 0 - 63 )
		*   ----  ----    ----  ----    ---1  ----    ----  ----  -> 'en passsant'      Mask: 0x00001000    Values: 0 / 1
		*   ----  ----    ----  ----    --1-  ----    ----  ----  -> 'pawn start'       Mask: 0x00002000    Values: 0 / 1  ( set only on 2 long moves )
		*   ----  ----    ----  ----    -1--  ----    ----  ----  -> 'castle'           Mask: 0x00004000    Vaules: 0 / 1
		*   ----  ----    ----  ----    1---  ----    ----  ----  -> 'capture'          Mask: 0x00008000    Vaules: 0 / 1
		*   ----  ----    ----  1111    ----  ----    ----  ----  -> 'promoted to'      Mask: 0x000f0000    Vaules: ( 0 - 12 )
		*   ----  ----    ---1  ----    ----  ----    ----  ----  -> 'is promote'       Mask: 0x00100000    Values: ( 0 - 1 )
		*/

#if defined(CHS_MOVE_STORE_EXTRA_INFO)
		char from[2] = { 0 };
		char to[2] = { 0 };
		bool pawnStart = false;
		bool castle = false;
		bool enp = false;
		bool capture = false;
		bool promoted = false;
		PieceType promotedTo = 0;
#endif

		friend std::ostream& operator<<(std::ostream& stream, const Move& move);
	};

	struct MoveList
	{
#define MV_LIST_SIZE 256

		void push_back(const Move& move) { moves[count++] = move; }
		Move pop_back() { return moves[count--]; }

		auto begin() { return moves.begin(); }
		auto end() { return moves.begin() + count; }

		auto begin() const { return moves.begin(); }
		auto end() const { return moves.begin() + count; }

		void erase(std::array<Move, 256>::const_iterator it)
		{
			auto pos = it - begin();
			ET_DEBUG_ASSERT((size_t)pos < count);
			count--;
			// last element
			if (pos == count)
			{
				return;
			}
			auto diff = count - pos;
			memmove_s(&moves[pos], sizeof(Move) * diff, &moves[pos + 1], sizeof(Move)* diff);
		}

		template <typename integer>
		Move& operator[](integer i) { ET_DEBUG_ASSERT(i < count); return moves[i]; }

		template <typename integer>
		const Move& operator[](integer i) const { ET_DEBUG_ASSERT(i < count); return moves[i]; }

		size_t size() const { return count; }

	private:
		std::array<Move, MV_LIST_SIZE> moves;
		size_t count = 0;

#undef MV_LIST_SIZE
	};

	struct MoveMetaData
	{

		PieceType CapturedType() const { return (PieceType)((data & (0x0000000full << 32)) >> 32); }
		void CapturedType(PieceType type)
		{
			// clear type
			data = (data & ~(0x0000000full << 32));
			data |= ((uint64_t)type << 32);

#if defined(CHS_MOVE_STORE_EXTRA_INFO)
			capturedType = type;
#endif
		}

		int32_t Fifty() const { return (int32_t)(((data & (0x000007f0ull << 32)) >> 36)); }
		void Fifty(int32_t fifty)
		{
			// clear fifty
			data = (data & ~(0x000007f0ull << 32));
			data |= ((uint64_t)fifty << 36);

#if defined(CHS_MOVE_STORE_EXTRA_INFO)
			this->fifty = fifty;
#endif
		}

		int32_t EnPassantTile() const { return (int32_t)((data & (0x0000007full))); }
		void EnPassantTile(int32_t index)
		{
			// clear enp
			data = (data & ~(0x0000007full));
			data |= ((uint64_t)index);
#if defined(CHS_MOVE_STORE_EXTRA_INFO)
			auto [file, rank] = IndexToFileRank(index);
			enpassant[0] = file;
			enpassant[1] = rank;
#endif
		}

		int32_t CastlePerm() const { return (int32_t)((data & 0x00000780ull) >> 7); }
		void CastlePerm(int32_t perm)
		{
			// clear perm
			data = (data & ~0x00000780ull);
			data |= ((uint64_t)perm << 7);

#if defined(CHS_MOVE_STORE_EXTRA_INFO)
			castlePerms[0] = perm & CastleBlackKing;
			castlePerms[1] = perm & CastleBlackQueen;
			castlePerms[2] = perm & CastleWhiteKing;
			castlePerms[3] = perm & CastleWhiteQueen;
#endif
		}

		int32_t Full() const { return (int32_t)((data & 0x07fff800ull) >> 11); }
		void Full(int32_t full)
		{
			// clear perm
			data = (data & ~0x07fff800ull);
			data |= ((uint64_t)full << 11);

#if defined(CHS_MOVE_STORE_EXTRA_INFO)
			this->full = full;
#endif
		}

		uint64_t data = 0;
		/*
		*   0000  0000    0000  0000    0000  0000    0000  0000
		*   ----  ----    ----  ----    ----  ----    -111  1111  -> 'en passant tile'  Mask: 0x0000007f    Values: ( 0 - 64 )
		*   ----  ----    ----  ----    ----  -111    1---  ----  -> 'castle perm'      Mask: 0x00000780    Values: ( 0 - 15 )
		*   ----  -111    1111  1111    1111  1---    ----  ----  -> 'full moves'       Mask: 0x07fff800    Values: ( 1 - 65535 )
		* 
		*   next 32 bits
		* 
		*   0000  0000    0000  0000    0000  0000    0000  0000
		*   ----  ----    ----  ----    ----  ----    ----  1111  -> 'captured piece'   Mask: 0x0000000f    Values: ( 0 - 64 )
		*   ----  ----    ----  ----    ----  -111    1111  ----  -> 'fifty move'       Mask: 0x000007f0    Vaules: ( 0 - 100 )  
		*/
#if defined(CHS_MOVE_STORE_EXTRA_INFO)
		char enpassant[2] = { 0 };
		bool castlePerms[4] = {};
		int32_t fifty = 0;
		int32_t full = 0;
		PieceType capturedType = 0;
#endif
	};
}

namespace std
{
	template <>
	struct hash<chs::Move>
	{
		size_t operator()(const chs::Move& move) const
		{
			auto hasher = hash<uint32_t>();
			return hasher(move.data);
		}
	};
}