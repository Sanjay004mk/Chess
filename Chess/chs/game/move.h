#pragma once

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
		*   ----  ----    ----  1111    ----  ----    ----  ----  -> 'promoted to'      Mask: 0x000f0000    Vaules: ( 0 - 12 )
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

	template <typename ostream>
	ostream& operator<<(ostream& stream, const Move& move)
	{
		stream << "Move: [ " << "from: " << IndexToStr(move.From()) << ", to: " << IndexToStr(move.To()) << ", en passant: " << move.EnPassant();
		stream << ", castle: " << move.Castle() << ", capture: " << move.Capture();
		move.Valid() ? stream << " (Valid)" : stream << " (Invalid)";
		stream << " ]";
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
			auto hasher = hash<uint32_t>();
			return hasher(move.data);
		}
	};
}