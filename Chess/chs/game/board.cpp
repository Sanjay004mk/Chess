#include <iostream>
#include "Entropy/EntropyUtils.h"

#include "board.h"

namespace chs
{
	static constexpr size_t num_tile_hashes = 63 | (15 << 6);
	static constexpr size_t num_enp_hashes = 63;
	static constexpr size_t num_castle_hashes = 15;
	static constexpr size_t num_turn_hashes = 2;
	static size_t PRE_COMPUTED_TILE_HASHES[num_tile_hashes] = {};
	static size_t PRE_COMPUTED_ENP_HASHES[num_enp_hashes] = {};
	static size_t PRE_COMPUTED_CASTLE_HASHES[num_castle_hashes] = {};
	static size_t PRE_COMPUTED_TURN_HASHES[num_turn_hashes] = {};

	void PreComputeBoardHashes()
	{
		auto hasher = std::hash<size_t>();

		for (size_t i = 0; i < num_tile_hashes; i++)
			PRE_COMPUTED_TILE_HASHES[i] = hasher(i);

		for (size_t i = 0; i < num_enp_hashes; i++)
			PRE_COMPUTED_ENP_HASHES[i] = hasher(i);

		for (size_t i = 0; i < num_castle_hashes; i++)
			PRE_COMPUTED_CASTLE_HASHES[i] = hasher(i);

		for (size_t i = 0; i < num_turn_hashes; i++)
			PRE_COMPUTED_TURN_HASHES[i] = hasher(i);
	}

	void EmptyMoves(const Board*, int32_t, std::vector<Move>&)
	{
	}

	GetMoveFn GetMoves[13] =
	{
		EmptyMoves,

		PawnMoves,
		PawnMoves,

		RookMoves,
		RookMoves,

		KnightMoves,
		KnightMoves,

		BishopMoves,
		BishopMoves,

		QueenMoves,
		QueenMoves,

		KingMoves,
		KingMoves,
	};

	// piece iterator
	void PieceIterator::Inc()
	{
		// if we are the last tile, return end()
		if (index >= 63)
		{
			index = 64;
			return;
		}

		while (!tiles[++index])
			if (index >= 63)
				break;

		// we broke from while loop at index = 63 so check if tiles[63] is valid, if not return end()
		if (!tiles[index])
			index = 64;
	}

	void PieceIterator::Dec()
	{
		// if we are the first tile, return end()
		if (index <= 0)
		{
			index = 64;
			return;
		}

		while (!tiles[--index])
			if (index <= 0)
				break;

		// we broke from while loop at index = 0 so check if tiles[0] is valid, if not return end()
		if (!tiles[index])
			index = 64;

	}

	PieceRenderInfo PieceIterator::Deref() const
	{
		if (index < 64 && index > -1)
			return { (int32_t)tiles[index], GetPositionFromIndex(index) };
		else
			return {};
	}

	bool PieceIterator::Valid() const
	{
		if (index < 64 && index > -1)
			return tiles[index];
		else
			return false;
	}

	bool Move::Valid() const
	{
		if (!data)
			return false;

		auto from = From();
		auto to = To();
		auto enPassant = EnPassant();
		auto castle = Castle();
		auto capture = Capture();
		auto promote = PromotedTo();

		if (from > 63 || from < 0 || to > 63 || to < 0)
			return false;

		if (enPassant)
			if (to < 16 || (to > 23 && to < 33) || to > 55)
				return false;

		if (castle)
		{
			if (!(from == 4 || from == 60))
				return false;

			if (!(to == 2 || to == 6 || to == 58 || to == 62))
				return false;
		}

		return true;
	}

	Board::Board(std::string_view fen_string)
	{
		LoadFromFen(fen_string);
		hashKey = hash();
	}

	void Board::LoadFromFen(std::string_view fen_string)
	{
		size_t i = 0;

		// board pieces
		int32_t pieceIndex = 55;
		while (fen_string[i] != ' ' && fen_string[i] != 0)
		{
			char c = fen_string[i];
			if (std::isalpha(c))
			{
				pieceIndex++;
				auto piece = CharToPiece[c];
				if (piece)
					AddPiece(pieceIndex, piece);
			}
			else
			{
				if (std::isdigit(c))
					pieceIndex += c - '0';
				else //if (c == '/')
					pieceIndex -= 16;
			}
			i++;
		}
		ET_DEBUG_ASSERT(fen_string[i] == ' ' || fen_string[i] == 0);
		// if fen string has only pieces
		if (fen_string[i] == 0)
			return;

		turn = (fen_string[++i] == 'w') ? White : Black;

		// castling
		i += 2;
		if (fen_string[i] != '-')
		{
			while (fen_string[i] != ' ')
			{
				switch (fen_string[i])
				{
				case 'Q': castlePermission |= CastleWhiteQueen; break;
				case 'q': castlePermission |= CastleBlackQueen; break;
				case 'K': castlePermission |= CastleWhiteKing; break;
				case 'k': castlePermission |= CastleBlackKing; break;
				}
				i++;
			}
			// white space
			i++;
		}
		else
			// '-' and white space
			i += 2;

		// en passant
		if (fen_string[i] != '-')
		{
			enPassant = FileRankToIndex(fen_string[i], fen_string[i + 1]);
			// algebra move and white space
			i += 3;
		}
		else
			// '-' and white space
			i += 2;

		// 50 moves and full moves
		char str[3] = { fen_string[i], fen_string[++i], 0 };
		if (fen_string[i++] == ' ')
			str[1] = 0;
		else
			i++;

		fiftyMove = std::atoi(str);

		str[0] = fen_string[i];

		if (i < fen_string.size() - 1)
			str[1] = fen_string[i + 1];
		else
			str[1] = 0;

		fullMoves = std::atoi(str);

	}

	std::string Board::GetFEN() const
	{
		std::string fen{};

		for (int32_t i = 7; i > -1; i--)
		{
			int32_t empty = 0;
			for (int32_t j = 0; j < 8; j++)
			{
				auto piece = tiles[(i * 8) + j];
				if (piece)
				{
					if (empty)
						fen += std::to_string(empty);
					fen += PieceToChar[piece];
					empty = 0;
				}
				else
					empty++;
			}
			if (empty)
				fen += std::to_string(empty);

			if (i != 0)
				fen += '/';
		}
		fen += ' ';
		fen += turn ? 'w' : 'b';
		fen += ' ';

		if (castlePermission & CastleWhiteKing)
			fen += 'K';
		if (castlePermission & CastleWhiteQueen)
			fen += 'Q';
		if (castlePermission & CastleBlackKing)
			fen += 'k';
		if (castlePermission & CastleBlackQueen)
			fen += 'q';

		if (fen.back() != ' ')
			fen += ' ';
		else
			fen += "- ";

		if (enPassant > -1 && enPassant < 64)
			fen += IndexToStr(enPassant);
		else
			fen += '-';

		fen += ' ';
		fen += std::to_string(fiftyMove);
		
		fen += ' ';
		fen += std::to_string(fullMoves);

		return fen;
	}

	bool Board::Valid() const
	{
		if (pieces[BlackKing].count != 1 || pieces[WhiteKing].count != 1)
			return false;

		std::vector<int32_t> piecesCheck[13];
		for (int32_t i = 0; i < 64; i++)
		{
			if (tiles[i])
			{
				auto piece = tiles[i];
				piecesCheck[piece].push_back(i);

				if (!IsKing(piece))
				{
					auto c = GetColor(piece);

					if (IsMajor[piece])
						if (!GetBit(majorBitBoard[c], i))
							return false;
					else if (IsMinor[piece])
						if (!GetBit(minorBitBoard[c], i))
							return false;
					else if (!GetBit(pawnBitBoard[c], i))
						return false;
				}
			}
		}

		for (size_t piece = 1; piece < 13; piece++)
		{
			if (pieces[piece].count != (uint32_t)piecesCheck[piece].size())
				return false;

			for (size_t i = 0; i < piecesCheck[piece].size(); i++)
			{
				bool found = false;

				for (size_t j = 0; j < piecesCheck[piece].size(); j++)
				{
					if (pieces[piece].positions[i] == piecesCheck[piece][j])
					{
						found = true;
						break;
					}
				}

				if (!found)
					return false;
			}
		}

		return true;
	}

	size_t Board::PerftTest(int32_t& capture, int32_t& ep, int32_t& castles, int32_t& prom, std::vector<std::vector<Move>>& depth_moves, int32_t depth)
	{
		if (depth <= 0)
			return 1ull;

		size_t nodes = 0;

		depth_moves[depth].clear();
		depth_moves[depth - 1].clear();

		GetAllMoves(depth_moves[depth], turn);

		for (auto move : depth_moves[depth])
		{
			ET_DEBUG_ASSERT(move.Valid());
			MovePiece(move);

			if (!IsAttacked(pieces[BlackKing + GetOppColor(turn)].positions[0], turn))
			{
				nodes += PerftTest(capture, ep, castles, prom, depth_moves, depth - 1);

				if (move.Capture())
					capture++;

				if (move.EnPassant())
					ep++;

				if (move.Castle())
					castles++;

				if (move.PromotedTo())
					prom++;
			}

			Revert(move);

			depth_moves[depth - 1].clear();
		}

		return nodes;
	}

	void Board::PerftRoot(int32_t depth)
	{
		ET_LOG_INFO("hash: 0x{:x}", hashKey);
		et::Timer t;
		int32_t captures = 0, ep = 0,castles = 0, prom = 0;

		size_t nodes = 0;

		auto moves = GetAllMoves(turn);
		// pre allocate memory for storing all moves
		std::vector<std::vector<Move>> perft_moves(depth);
		for (auto& moves : perft_moves)
			moves.reserve(256);

		for (auto move : moves)
		{
			MovePiece(move);

			if (!IsAttacked(pieces[BlackKing + GetOppColor(turn)].positions[0], turn))
			{
				auto temp = PerftTest(captures, ep, castles, prom, perft_moves, depth - 1);
				nodes += temp;

				if (move.Capture())
					captures++;

				if (move.EnPassant())
					ep++;

				if (move.Castle())
					castles++;

				if (move.PromotedTo())
					prom++;
				
				ET_LOG_INFO("{}{} : {}", IndexToStr(move.From()), IndexToStr(move.To()), temp);
			}
			Revert(move);
		}
		ET_LOG_INFO("Nodes: {}", nodes);
		ET_LOG_INFO("Captures: {}, EnPassant: {}, Castles: {}, Promotions: {}", captures, ep, castles, prom);
		ET_LOG_INFO("Time: {} s", t.Elapsed());
		ET_LOG_INFO("hash: 0x{:x}", hashKey);
	}

	bool Board::AddPiece(int32_t index, PieceType piece)
	{
		if (tiles[index])
			return false;

		tiles[index] = piece;
		pieces[piece].positions[pieces[piece].count++] = index;

		// add to bit board
		if (!IsKing(piece))
		{
			auto c = GetColor(piece);
			if (IsMajor[piece])
				SetBit(majorBitBoard[c], 1, index);
			else if (IsMinor[piece])
				SetBit(minorBitBoard[c], 1, index);
			else
				SetBit(pawnBitBoard[c], 1, index);
		}
		return true;
	}

	bool Board::RemovePiece(int32_t index, PieceType* piece)
	{
		auto p = tiles[index];
		if (!p)
			return false;

		if (piece)
			*piece = p;

		tiles[index] = 0;

		auto it = std::find(pieces[p].positions.begin(), pieces[p].positions.end(), index);
		ET_DEBUG_ASSERT(it != pieces[p].positions.end());

		auto pos = (it - pieces[p].positions.begin());

		// 'index' is not the last element
		// assign the value of the last element to the removed element position
		if (pos != (pieces[p].count - 1))
			pieces[p].positions[pos] = pieces[p].positions[pieces[p].count - 1];

		pieces[p].count--;

		// clear from bit board
		if (!IsKing(p))
		{
			auto c = GetColor(p);
			if (IsMajor[p])
				SetBit(majorBitBoard[c], 0, index);
			else if (IsMinor[p])
				SetBit(minorBitBoard[c], 0, index);
			else
				SetBit(pawnBitBoard[c], 0, index);
		}
		return true;
	}

	bool Board::ShiftPiece(int32_t from, int32_t to, PieceType* piece)
	{
		auto p = tiles[from];
		if (!p)
			return false;

		if (tiles[to])
			return false;

		if (piece)
			*piece = p;

		tiles[from] = 0;
		tiles[to] = p;

		auto it = std::find(pieces[p].positions.begin(), pieces[p].positions.end(), from);
		ET_DEBUG_ASSERT(it != pieces[p].positions.end());

		auto pos = (it - pieces[p].positions.begin());
		pieces[p].positions[pos] = to;

		// update bit board
		if (!IsKing(p))
		{
			auto c = GetColor(p);
			if (IsMajor[p])
			{
				SetBit(majorBitBoard[c], 0, from);

				SetBit(majorBitBoard[c], 1, to);
			}
			else if (IsMinor[p])
			{
				SetBit(minorBitBoard[c], 0, from);

				SetBit(minorBitBoard[c], 1, to);
			}
			else
			{
				SetBit(pawnBitBoard[c], 0, from);

				SetBit(pawnBitBoard[c], 1, to);
			}
		}

		return true;
	}

	std::unordered_map<glm::vec2, Move> Board::GetMoveTiles(const glm::vec2& position)
	{
		auto t = turn;
		if (!InsideBoard(position))
			return {};

		auto idx = GetIndexFromPosition(position);
		if (GetColor(tiles[idx]) != t)
			return {};

		auto moves = GetMoveTiles(idx);
		// check if move results in current side ending up in check
		auto copy = moves;
		for (auto& move : copy)
		{
			auto c_move = move;
			MovePiece(move);

			// move results in player exposing king to check
			if (IsAttacked(pieces[BlackKing + t].positions[0], GetOppColor(t)))
			{
				auto it = std::find(moves.begin(), moves.end(), c_move);
				ET_DEBUG_ASSERT(it != moves.end());
				moves.erase(it);
			}

			Revert(move);
		}

		// remove promotion moves to allow the player to select which piece to promote to
		for (auto& move : moves)
		{
			if (move.IsPromoted())
			{
				// also clears isPromoted
				move.PromotedTo(0);
				// so set it to true again
				move.IsPromoted(true);
			}
		}

		std::unordered_map<glm::vec2, Move> tiles;
		for (auto& move : moves)
			tiles[(GetPositionFromIndex(move.To()))] = move;

		return tiles;
	}

	size_t def_vec_size[13] = { 0, 10, 10, 14, 14, 8, 8, 14, 14, 28, 28, 8, 8 };

	std::vector<Move> Board::GetMoveTiles(int32_t position)
	{
		std::vector<Move> moves;
		moves.reserve(def_vec_size[tiles[position]]);
		GetMoves[tiles[position]](this, position, moves);
		return moves;
	}

	static inline uint32_t castle_mask(int32_t index)
	{
		// rooks
		if (index == 0)
			return ~CastleWhiteQueen;
		else if (index == 7)
			return ~CastleWhiteKing;
		else if (index == 56)
			return ~CastleBlackQueen;
		else if (index == 63)
			return ~CastleBlackKing;
		// kings
		else if (index == 4)
			return ~(CastleWhiteKing | CastleWhiteQueen);
		else if (index == 60)
			return ~(CastleBlackKing | CastleBlackQueen);

		return 15u;
	}

	bool Board::MovePiece(Move& move)
	{
		ET_DEBUG_ASSERT(move.Valid());

		move.Fifty(fiftyMove);
		move.Full(fullMoves);
		move.EnPassantTile(enPassant);
		move.CastlePerm(castlePermission);

		fiftyMove++;

		turn = GetOppColor(turn);

		if (turn)
			fullMoves++;

		if (move.Capture())
		{
			fiftyMove = 0;
			auto cap_tile = move.EnPassant() ? EnPassantToPiece(enPassant) : move.To();
			PieceType rem_piece = 0;
			if (!RemovePiece(cap_tile, &rem_piece))
				return false;

			// castle rights
			castlePermission &= castle_mask(move.To());

			capturedTiles.push_back({ cap_tile, rem_piece });
			move.CapturedType(rem_piece);
		}
		else if (move.Castle())
		{
			bool queenSide = ((move.To() % 8) < 4);
			ClearCastle(castlePermission, GetColor(tiles[move.From()]), queenSide);
			int32_t rook_from = move.To() + (queenSide ? -2 : 1);
			int32_t rook_to = move.To() + (queenSide ? 1 : -1);
			if (!ShiftPiece(rook_from, rook_to))
				return false;

		}

		if (IsPawn(tiles[move.From()]))
			fiftyMove = 0;

		// castle rights 
		castlePermission &= castle_mask(move.From());

		if (move.PawnStart())
			enPassant = PieceToEnPassant(move.To());
		else
			enPassant = 64;

		if (!ShiftPiece(move.From(), move.To()))
			return false;

		if (move.PromotedTo())
		{
			ET_DEBUG_ASSERT(move.IsPromoted() && IsPawn(tiles[move.To()]));

			if (!RemovePiece(move.To()))
				return false;
			if (!AddPiece(move.To(), move.PromotedTo()))
				return false;
		}

		hashKey = hash();

		ET_DEBUG_ASSERT(Valid());

		return true;
	}

	bool Board::Revert(Move move)
	{
		if (!move.Valid())
			return false;

		fiftyMove = move.Fifty();
		fullMoves = move.Full();
		enPassant = move.EnPassantTile();
		castlePermission = move.CastlePerm();
		turn = GetOppColor(turn);

		if (!ShiftPiece(move.To(), move.From()))
			return false;

		if (move.Capture())
		{
			auto cap_tile = move.EnPassant() ? EnPassantToPiece(enPassant) : move.To();
			PieceType rem_piece = move.CapturedType();
			if (!AddPiece(cap_tile, rem_piece))
				return false;

			std::pair<int32_t, PieceType> t = { cap_tile, rem_piece };
			auto it = std::find(capturedTiles.begin(), capturedTiles.end(), t);
			ET_DEBUG_ASSERT(it != capturedTiles.end());
			capturedTiles.erase(it);
		}
		else if (move.Castle())
		{
			bool queenSide = ((move.To() % 8) < 4);
			int32_t rook_from = move.To() + (queenSide ? -2 : 1);
			int32_t rook_to = move.To() + (queenSide ? 1 : -1);
			if (!ShiftPiece(rook_to, rook_from))
				return false;

		}

		if (move.PromotedTo())
		{
			ET_DEBUG_ASSERT(move.IsPromoted());

			if (!RemovePiece(move.From()))
				return false;
			if (!AddPiece(move.From(), BlackPawn + turn))
				return false;
		}

		hashKey = hash();

		ET_DEBUG_ASSERT(Valid());

		return true;
	}

	bool Board::MakeMove(Move move, bool& shouldPromote)
	{
		if (!MovePiece(move))
			return false;

		if (move.IsPromoted())
			shouldPromote = true;
		else
			shouldPromote = false;

		playedMoves.push_back(move);

		// if king is attacked, check if that side has a valid moves that 
		// results in the king not being attack ( IsInCheck() )
		// store all the valid moves in a vector and only allow moves that are 
		// in this vector when the player tries to move
		if (inCheck = IsAttacked(pieces[BlackKing + turn].positions[0], GetOppColor(turn)))
		{
			if (IsInCheck(turn, checkValidMoves))
			{
				auto win_str = turn ? "Black wins" : "White wins";
				ET_LOG_INFO("CHECKMATE! {}", win_str);
			}
			else
				ET_LOG_INFO("CHECKMATE!");
		}
		else
		{
			inCheck = false;
			checkValidMoves.clear();
		}

		return true;
	}

	bool Board::Promote(PieceType piece)
	{
		if (IsKing(piece) || !piece)
			return false;

		Move& move = playedMoves.back();
		if (!move.IsPromoted())
			return false;

		PieceType p = 0;
		if (!RemovePiece(move.To(), &p))
			return false;

		ET_DEBUG_ASSERT(IsPawn(p));

		if (!AddPiece(move.To(), piece))
			return false;

		move.PromotedTo(piece);

		ET_DEBUG_ASSERT(Valid());

		return true;
	}

	bool Board::Undo()
	{
		if (playedMoves.empty())
			return false;

		if (!Revert(playedMoves.back()))
			return false;

		playedMoves.pop_back();

		// reset checkmate
		inCheck = false;
		checkValidMoves.clear();

		return true;
	}

	bool Board::IsInCheck(Color c, std::vector<Move>& validMoves)
	{
		validMoves.clear();
		auto moves = GetAllMoves(c);

		bool check = true;
		// for every possible move the given side can make,
		// check if there is a move where the king is not attacked
		// if there is, the side is not in check
		for (size_t i = 0; i < moves.size(); i++)
		{
			auto move = moves[i];
			MovePiece(moves[i]);

			if (!IsAttacked(pieces[BlackKing + c].positions[0], GetOppColor(c) ))
			{
				check = false;
				// moves[i] gets filled with extra information by 'MovePiece'
				// so use a copy of it without the extra info
				validMoves.push_back(move);
			}

			Revert(moves[i]);
		}

		return check;
	}

	std::vector<Move> Board::GetAllMoves(Color side)
	{
		std::vector<Move> moves;
		moves.reserve(256);

		GetAllMoves(moves, side);

		return moves;		
	}

	void Board::GetAllMoves(std::vector<Move>& moves, Color side)
	{
		for (uint32_t i = 1; i < 13; i++)
		{
			if (GetColor(i) != side)
				continue;

			for (uint32_t count = 0; count < pieces[i].count; count++)
				GetMoves[i](this, pieces[i].positions[count], moves);
		}
	}

	PieceType Board::GetTile(const glm::vec2& tile)
	{
		if (!InsideBoard(tile))
			return 0;
		return GetTile(GetIndexFromPosition(tile));
	}

	bool Board::IsPiece(const glm::vec2& tile)
	{
		if (!InsideBoard(tile))
			return false;
		return IsPiece(GetIndexFromPosition(tile));
	}

	PieceType Board::GetTile(int32_t index)
	{
		ET_DEBUG_ASSERT(index > -1 && index < 64);
		return tiles[index];
	}

	bool Board::IsPiece(int32_t index)
	{
		ET_DEBUG_ASSERT(index > -1 && index < 64);
		return tiles[index];
	}

	inline void hash_combine(size_t& seed, size_t other)
	{
		seed ^= other + 0x9e3779b9 + (seed << 6) + (seed >> 2);
	}

	size_t Board::hash() const
	{
		size_t seed = 0;
		for (int32_t i = 0; i < 64; i++)
			if (tiles[i])
				hash_combine(seed, PRE_COMPUTED_TILE_HASHES[tiles[i] | (i << 3)]);
			
		hash_combine(seed, PRE_COMPUTED_ENP_HASHES[enPassant]);
		hash_combine(seed, PRE_COMPUTED_CASTLE_HASHES[castlePermission]);
		hash_combine(seed, PRE_COMPUTED_TURN_HASHES[turn]);

		return seed;
	}

	struct Position
	{
		Position(int32_t index) : position(GetPositionFromIndex<int32_t>(index)) {}

		bool Move(int32_t x, int32_t y, int32_t& index) const
		{
			auto newPos = position + glm::ivec2(x, y);

			if (!InsideBoard(newPos))
				return false;

			index = GetIndexFromPosition<int32_t>(newPos);
			return true;
		}

		static bool InsideBoard(const glm::ivec2& pos)
		{
			return pos.x > 0 && pos.x < 9 &&
				   pos.y > 0 && pos.y < 9;
		}

		glm::ivec2 position;
	};

	// side capture / en passant capture
	static void side_capture (const Position& position, int32_t forward, const PieceType tiles[64], int32_t enPassant, bool isWhite, bool prom, int32_t index, std::vector<Move>& moves, int32_t side, int32_t& index__)
	{
		int32_t enP = 0;
		if (position.Move(side, forward, index__))
		{
			bool valid = true;
			if (tiles[index__])
			{
				if (!(IsWhite(tiles[index__]) ^ isWhite))
					valid = false;
			}
			else if (enPassant == index__)
			{
				auto idx = EnPassantToPiece(enPassant);
				if (!(IsWhite(tiles[idx]) ^ isWhite))
					valid = false;

				enP = 1;
			}
			else
				valid = false;

			if (valid)
			{
				if (prom)
				{
					moves.push_back(Move(index, index__, 1, 0, 0, 0, BlackRook + isWhite));
					moves.push_back(Move(index, index__, 1, 0, 0, 0, BlackKnight + isWhite));
					moves.push_back(Move(index, index__, 1, 0, 0, 0, BlackBishop + isWhite));
					moves.push_back(Move(index, index__, 1, 0, 0, 0, BlackQueen + isWhite));
				}
				else
					moves.push_back(Move(index, index__, 1, 0, enP, 0, 0));
			}

		}
	};

	void PawnMoves(const Board* board, int32_t index, std::vector<Move>& moves)
	{
		auto& piece = board->tiles[index];
		ET_DEBUG_ASSERT(IsPawn(piece));

		bool isWhite = IsWhite(piece);
		auto position = Position(index);
		std::array<int32_t, 4> possible = { -1 };

		bool start =  isWhite ? (position.position.y == 2) : (position.position.y == 7);
		bool prom = isWhite ? (position.position.y == 7) : (position.position.y == 2);

		int32_t forward = isWhite ? 1 : -1;
		
		// forward one
		if (position.Move(0, forward, possible[0]))
		{
			bool valid = true;
			// tile is not empty
			if (board->tiles[possible[0]])
					valid = false;

			if (valid)
			{
				if (prom)
				{
					moves.push_back(Move(index, possible[0], 0, 0, 0, 0, BlackRook + isWhite));
					moves.push_back(Move(index, possible[0], 0, 0, 0, 0, BlackKnight + isWhite));
					moves.push_back(Move(index, possible[0], 0, 0, 0, 0, BlackBishop + isWhite));
					moves.push_back(Move(index, possible[0], 0, 0, 0, 0, BlackQueen + isWhite));
				}
				else
					moves.push_back(Move(index, possible[0], 0, 0, 0, 0, 0));
			}
		}
		// double forward
		if (start)
		{
			bool valid = true;
			possible[1] = index + (16 * (isWhite ? 1 : -1));

			if (board->tiles[possible[0]] || board->tiles[possible[1]])
				valid = false;

			if (valid)
				moves.push_back(Move(index, possible[1], 0, 1, 0, 0, 0));
		}

		

		side_capture(position, forward, board->tiles, board->enPassant, isWhite, prom, index, moves, 1, possible[2]);
		side_capture(position, forward, board->tiles, board->enPassant, isWhite, prom, index, moves, -1, possible[3]);
	}

	void Slide(const Board* board, int32_t index, const glm::ivec2& direction, std::vector<Move>& moves)
	{
		auto start_index = index;
		Position position(index);
		bool isWhite = IsWhite(board->tiles[index]);

		while (position.Move(direction.x, direction.y, index))
		{
			position.position += direction;

			int32_t capture = 0;
			if (board->tiles[index])
			{
				if (!(IsWhite(board->tiles[index]) ^ isWhite))
					return;
				capture = 1;
			}
			moves.push_back(Move(start_index, index, capture));

			if (capture)
				return;
		}
	}

	void RookMoves(const Board* board, int32_t index, std::vector<Move>& moves)
	{
		Slide(board, index, glm::ivec2(1,  0), moves);
		Slide(board, index, glm::ivec2(-1, 0), moves);
		Slide(board, index, glm::ivec2(0,  1), moves);
		Slide(board, index, glm::ivec2(0, -1), moves);
	}

	static void load_knight(const Position& position, const PieceType tiles[64], int32_t index, std::vector<Move>& moves, bool isWhite, const glm::ivec2& offs)
	{
		int32_t end_index = -1;
		if (position.Move(offs.x, offs.y, end_index))
		{
			int32_t capture = 0;
			if (tiles[end_index])
			{
				capture = 1;
				if (!(IsWhite(tiles[end_index]) ^ isWhite))
					return;
			}
			moves.push_back(Move(index, end_index, capture));
		}
	};

	void KnightMoves(const Board* board, int32_t index, std::vector<Move>& moves)
	{
		Position position(index);

		bool isWhite = IsWhite(board->tiles[index]);

		load_knight(position, board->tiles, index, moves, isWhite, glm::ivec2( 1,  2));
		load_knight(position, board->tiles, index, moves, isWhite, glm::ivec2( 1, -2));
		load_knight(position, board->tiles, index, moves, isWhite, glm::ivec2(-1,  2));
		load_knight(position, board->tiles, index, moves, isWhite, glm::ivec2(-1, -2));

		load_knight(position, board->tiles, index, moves, isWhite, glm::ivec2( 2, -1));
		load_knight(position, board->tiles, index, moves, isWhite, glm::ivec2( 2,  1));
		load_knight(position, board->tiles, index, moves, isWhite, glm::ivec2(-2, -1));
		load_knight(position, board->tiles, index, moves, isWhite, glm::ivec2(-2,  1));
	}

	void BishopMoves(const Board* board, int32_t index, std::vector<Move>& moves)
	{
		Slide(board, index, glm::ivec2(1,  1), moves);
		Slide(board, index, glm::ivec2(-1, 1), moves);
		Slide(board, index, glm::ivec2(-1,-1), moves);
		Slide(board, index, glm::ivec2(1, -1), moves);
	}

	void QueenMoves(const Board* board, int32_t index, std::vector<Move>& moves)
	{
		Slide(board, index, glm::ivec2(1,   0), moves);
		Slide(board, index, glm::ivec2(-1,  0), moves);
		Slide(board, index, glm::ivec2(0,   1), moves);
		Slide(board, index, glm::ivec2(0,  -1), moves);
		Slide(board, index, glm::ivec2(1,   1), moves);
		Slide(board, index, glm::ivec2(-1,  1), moves);
		Slide(board, index, glm::ivec2(-1, -1), moves);
		Slide(board, index, glm::ivec2(1,  -1), moves);
	}

	static void load_king(const Position& position, const PieceType tiles[64], int32_t index, std::vector<Move>& moves, bool isWhite, const glm::ivec2& offs)
	{
		int32_t end_index = -1;
		if (position.Move(offs.x, offs.y, end_index))
		{
			int32_t capture = 0;
			if (tiles[end_index])
			{
				capture = 1;
				if (!(IsWhite(tiles[end_index]) ^ isWhite))
					return;
			}
			moves.push_back(Move(index, end_index, capture));
		}
	};

	static void check_castle(const Board* board, int32_t castlePermission, const PieceType tiles[64], int32_t index, std::vector<Move>& moves, bool queenside)
	{
		if (CanCastle(castlePermission, GetColor(tiles[index]), queenside))
		{
			int32_t dir_x = queenside ? -1 : 1;
			int32_t x = dir_x;
			int32_t king = index + (queenside ? -2 : 2);
			static std::vector<int32_t> castle_tiles = { 1, 2 };
			if (queenside)
			{
				castle_tiles[0] = index - 1;
				castle_tiles[1] = index - 2;
				// queen side a2 / h2 can be attacked for castling
				// so check if it isn't empty
				if (tiles[index - 3])
					return;
			}
			else
			{
				castle_tiles[0] = index + 1;
				castle_tiles[1] = index + 2;
			}

			for (auto tile : castle_tiles)
			{
				if (tiles[tile] || board->IsAttacked(tile, ~(GetColor(tiles[index])) & 1u))
					return;
			}


			moves.push_back(Move(index, king, 0, 0, 0, 1, 0));
		}
	};

	void KingMoves(const Board* board, int32_t index, std::vector<Move>& moves)
	{
		Position position(index);
		

		bool isWhite = IsWhite(board->tiles[index]);

		load_king(position, board->tiles, index, moves, isWhite, glm::ivec2( 1,  0));
		load_king(position, board->tiles, index, moves, isWhite, glm::ivec2(-1,  0));
		load_king(position, board->tiles, index, moves, isWhite, glm::ivec2( 0,  1));
		load_king(position, board->tiles, index, moves, isWhite, glm::ivec2( 0, -1));

		load_king(position, board->tiles, index, moves, isWhite, glm::ivec2( 1, -1));
		load_king(position, board->tiles, index, moves, isWhite, glm::ivec2( 1,  1));
		load_king(position, board->tiles, index, moves, isWhite, glm::ivec2(-1, -1));
		load_king(position, board->tiles, index, moves, isWhite, glm::ivec2(-1,  1));

		// castle
		{

			if (!board->IsAttacked(index, GetOppColorFromPiece(board->tiles[index])))
			{
				check_castle(board, board->castlePermission, board->tiles, index, moves, true);
				check_castle(board, board->castlePermission, board->tiles, index, moves, false);
			}
		}
	}

	static void slide_check(const Position& position, int32_t tile_index, const PieceType tiles[64], bool& attacked, Color by, const glm::ivec2& direction, PieceType other)
	{
		auto copy_pos = position;
		// check king
		{
			if (copy_pos.Move(direction.x, direction.y, tile_index))
			{
				if (tiles[tile_index])
				{
					if (((GetColor(tiles[tile_index]) == by)) &&
						(SamePiece(tiles[tile_index], other) || IsQueen(tiles[tile_index]) || IsKing(tiles[tile_index])))
						attacked = true;
					return;
				}
				copy_pos.position += direction;
			}
		}

		while (copy_pos.Move(direction.x, direction.y, tile_index))
		{
			if (tiles[tile_index])
			{
				if ((GetColor(tiles[tile_index]) == by) &&
					(SamePiece(tiles[tile_index], other) || IsQueen(tiles[tile_index])))
					attacked = true;
				return;
			}
			copy_pos.position += direction;
		}
	}


	bool Board::IsAttacked(int32_t index, Color by) const
	{
		ET_DEBUG_ASSERT(index > -1 || index < 64);

		int32_t tile_index = -1;
		Position position(index);
		int32_t pawn_offs = by == White ? -1 : 1;

		{
			// pawns
			static std::vector<int32_t> x = { 1, -1 };
			for (auto& __x : x)
			{
				if (position.Move(__x, pawn_offs, tile_index))
				{
					if (tiles[tile_index])
						if ((GetColor(tiles[tile_index]) == by) && 
							(IsPawn(tiles[tile_index]) || IsBishop(tiles[tile_index]) || IsQueen(tiles[tile_index]) || IsKing(tiles[tile_index])))
							return true;
				}

			}

			// knights
			static std::vector<glm::ivec2> dirs =
			{
				glm::ivec2( 1,  2),
				glm::ivec2(-1,  2),
				glm::ivec2( 1, -2),
				glm::ivec2(-1, -2),

				glm::ivec2( 2,  1),
				glm::ivec2(-2,  1),
				glm::ivec2( 2, -1),
				glm::ivec2(-2, -1),
			};
			for (auto& dir : dirs)
			{
				if (position.Move(dir.x, dir.y, tile_index))
				{
					if (tiles[tile_index])
						if ((GetColor(tiles[tile_index]) == by) &&
							(IsKnight(tiles[tile_index])))
							return true;
				}
			}
		}

		bool attacked = false;		

#define check(x, y) slide_check(position, tile_index, tiles, attacked, by, x, y); if (attacked) return true

		check(glm::ivec2( 1,  0), WhiteRook);
		check(glm::ivec2(-1,  0), WhiteRook);
		check(glm::ivec2( 0,  1), WhiteRook);
		check(glm::ivec2( 0, -1), WhiteRook);

		check(glm::ivec2( 1,  1), WhiteBishop);
		check(glm::ivec2( 1, -1), WhiteBishop);
		check(glm::ivec2(-1,  1), WhiteBishop);
		check(glm::ivec2(-1, -1), WhiteBishop);

#undef check

		return false;
	}

}