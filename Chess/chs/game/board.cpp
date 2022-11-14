#include <iostream>
#include "Entropy/EntropyUtils.h"

#include "board.h"

namespace chs
{
	void EmptyMoves(const Board*, int32_t, MoveList&)
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

	Board::Board(std::string_view fen_string, const BoardSpecification& specs)
		: specs(specs)
	{
		LoadFromFen(fen_string);
		hashKey = hash();
		playedMoves.reserve(2048);
		CalcMaterialScores();
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
		if (fen_string.size() == i)
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
			// algebra move
			i += 2;
		}
		else
			// '-'
			i++;

		if (fen_string.size() == i)
			return;

		// whitespace
		i++;

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

		// start from rank 8
		for (int32_t i = 7; i > -1; i--)
		{
			int32_t empty = 0;
			// file 'a' to 'h'
			for (int32_t j = 0; j < 8; j++)
			{
				auto piece = tiles[(i * 8) + j];
				if (piece)
				{
					// if there was empty space before this piece
					if (empty)
						fen += std::to_string(empty);
					fen += PieceToChar[piece];
					empty = 0;
				}
				else
					empty++;
			}
			// if entire rank was empty
			if (empty)
				fen += std::to_string(empty);

			// don't add '/' at the end of the string (rank 1)
			if (i != 0)
				fen += '/';
		}
		// turn
		fen += ' ';
		fen += turn ? 'w' : 'b';
		fen += ' ';

		// castle permission
		if (castlePermission & CastleWhiteKing)
			fen += 'K';
		if (castlePermission & CastleWhiteQueen)
			fen += 'Q';
		if (castlePermission & CastleBlackKing)
			fen += 'k';
		if (castlePermission & CastleBlackQueen)
			fen += 'q';

		// if there was a catle permission add only a white space else also add a hyphen
		if (fen.back() != ' ')
			fen += ' ';
		else
			fen += "- ";

		// en passant
		if (enPassant > -1 && enPassant < 64)
			fen += IndexToStr(enPassant);
		else
			fen += '-';

		// 50 moves
		fen += ' ';
		fen += std::to_string(fiftyMove);
		
		// full moves
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

	size_t Board::PerftTest(int32_t& capture, int32_t& ep, int32_t& castles, int32_t& prom, int32_t depth)
	{
		if (depth <= 0)
			return 1ull;

		size_t nodes = 0;

		auto moves = GetAllMoves(turn);

		for (auto move : moves)
		{
			ET_DEBUG_ASSERT(move.Valid());
			MovePiece(move);

			if (!IsAttacked(pieces[BlackKing + GetOppColor(turn)].positions[0], turn))
			{
				nodes += PerftTest(capture, ep, castles, prom, depth - 1);

				if (move.Capture())
					capture++;

				if (move.EnPassant())
					ep++;

				if (move.Castle())
					castles++;

				if (move.PromotedTo())
					prom++;
			}

			Revert();
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

		for (auto move : moves)
		{
			MovePiece(move);

			if (!IsAttacked(pieces[BlackKing + GetOppColor(turn)].positions[0], turn))
			{
				auto temp = PerftTest(captures, ep, castles, prom, depth - 1);
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
			Revert();
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

#define MOVE_FAIL fiftyMove = data.Fifty();\
				  fullMoves = data.Full();\
				  enPassant = data.EnPassantTile();\
				  castlePermission = data.CastlePerm();\
				  turn = GetOppColor(turn)

		MoveMetaData data;

		data.Fifty(fiftyMove);
		data.Full(fullMoves);
		data.EnPassantTile(enPassant);
		data.CastlePerm(castlePermission);

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
			{
				MOVE_FAIL;
				return false;
			}

			// castle rights
			castlePermission &= castle_mask(move.To());

			capturedTiles.push_back({ cap_tile, rem_piece });
			data.CapturedType(rem_piece);
		}
		else if (move.Castle())
		{
			bool queenSide = ((move.To() % 8) < 4);
			ClearCastle(castlePermission, GetColor(tiles[move.From()]), queenSide);
			int32_t rook_from = move.To() + (queenSide ? -2 : 1);
			int32_t rook_to = move.To() + (queenSide ? 1 : -1);
			if (!ShiftPiece(rook_from, rook_to))
			{
				MOVE_FAIL;
				return false;
			}

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
		{
			MOVE_FAIL;
			return false;
		}

		if (move.PromotedTo())
		{
			ET_DEBUG_ASSERT(move.IsPromoted() && IsPawn(tiles[move.To()]));

			if (!RemovePiece(move.To()))
			{
				MOVE_FAIL;
				return false;
			}
			if (!AddPiece(move.To(), move.PromotedTo()))
			{
				MOVE_FAIL;
				return false;
			}
		}

		poskeys.push_back(hashKey);
		playedMoves.push_back({ move, data });
		hashKey = hash();
		ply++;
		CalcMaterialScores();

		ET_DEBUG_ASSERT(Valid());

		return true;
	}

	bool Board::Revert()
	{
		auto [move, data] = playedMoves.back();
		ET_DEBUG_ASSERT(move.Valid());

		fiftyMove = data.Fifty();
		fullMoves = data.Full();
		enPassant = data.EnPassantTile();
		castlePermission = data.CastlePerm();
		turn = GetOppColor(turn);

		if (!ShiftPiece(move.To(), move.From()))
			return false;

		if (move.Capture())
		{
			auto cap_tile = move.EnPassant() ? EnPassantToPiece(enPassant) : move.To();
			PieceType rem_piece = data.CapturedType();
			ET_DEBUG_ASSERT(rem_piece);
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

		hashKey = poskeys.back();
		poskeys.pop_back();
		playedMoves.pop_back();
		ply--;
		CalcMaterialScores();

		ET_DEBUG_ASSERT(Valid());

		return true;
	}

	// called by 'TileManager'
	bool Board::MakeMove(Move move, bool& shouldPromote)
	{
		if (!MovePiece(move))
			return false;

		if (move.IsPromoted())
			shouldPromote = true;
		else
			shouldPromote = false;

		UpdateCheckmate();

		return true;
	}

	bool Board::Promote(PieceType piece)
	{
		if (IsKing(piece) || !piece)
			return false;

		auto& [move, data] = playedMoves.back();
		if (!move.IsPromoted())
			return false;

		PieceType p = 0;
		if (!RemovePiece(move.To(), &p))
			return false;

		ET_DEBUG_ASSERT(IsPawn(p));

		if (!AddPiece(move.To(), piece))
			return false;

		move.PromotedTo(piece);

		hashKey = hash();

		UpdateCheckmate();

		CalcMaterialScores();
		ET_DEBUG_ASSERT(Valid());

		return true;
	}

	bool Board::Undo()
	{
		if (playedMoves.empty())
			return false;

		if (!Revert())
			return false;

		// reset checkmate
		inCheck = checkmate = false;

		return true;
	}

	void Board::UpdateCheckmate()
	{
		// if king is attacked, check if that side has a valid moves that 
		// results in the king not being attack ( IsInCheck() )
		if (inCheck = IsAttacked(pieces[BlackKing + turn].positions[0], GetOppColor(turn)))
		{
			if (IsInCheck(turn))
			{
				auto win_str = turn ? "Black wins" : "White wins";
				ET_LOG_INFO("CHECKMATE! {}", win_str);
				checkmate = true;
			}
			else
				ET_LOG_INFO("CHECKMATE!");
		}
		else
		{
			inCheck = checkmate = false;
		}
	}

	bool Board::IsInCheck(Color c)
	{
		auto moves = GetAllMoves(c);

		// for every possible move the given side can make,
		// check if there is a move where the king is not attacked
		// if there is, the side is not in check
		for (size_t i = 0; i < moves.size(); i++)
		{
			MovePiece(moves[i]);

			if (!IsAttacked(pieces[BlackKing + c].positions[0], GetOppColor(c) ))
			{
				Revert();
				return false;
			}

			Revert();
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

			Revert();
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

	MoveList Board::GetMoveTiles(int32_t position)
	{
		MoveList moves;
		GetMoves[tiles[position]](this, position, moves);
		return moves;
	}

	MoveList Board::GetAllMoves(Color side)
	{
		MoveList moves;

		GetAllMoves(moves, side);

		return moves;		
	}

	void Board::GetAllMoves(MoveList& moves, Color side)
	{
		for (uint32_t i = 1 + side; i < 13; i += 2)
		{
			for (uint32_t count = 0; count < pieces[i].count; count++)
				GetMoves[i](this, pieces[i].positions[count], moves);
		}
	}

	MoveList Board::GetAllLegalMoves(Color side)
	{
		MoveList moves;
		GetAllLegalMoves(moves, side);
		return moves;
	}

	void Board::GetAllLegalMoves(MoveList& moves, Color side)
	{
		GetAllMoves(moves, side);
		// check if move results in current side ending up in check
		auto copy = moves;
		for (auto& move : copy)
		{
			auto c_move = move;
			MovePiece(move);

			// move results in player exposing king to check
			if (IsAttacked(pieces[BlackKing + side].positions[0], GetOppColor(side)))
			{
				auto it = std::find(moves.begin(), moves.end(), c_move);
				ET_DEBUG_ASSERT(it != moves.end());
				moves.erase(it);
			}

			Revert();
		}
	}

	MoveList Board::GetAllCaptureMoves(Color side)
	{
		MoveList moves;
		GetAllCaptureMoves(moves, side);
		return moves;
	}

	void Board::GetAllCaptureMoves(MoveList& moves, Color side)
	{
		GetAllLegalMoves(moves, side);
		auto copy = moves;
		for (auto& move : copy)
		{
			if (!move.Capture())
			{
				auto it = std::find(moves.begin(), moves.end(), move);
				ET_DEBUG_ASSERT(it != moves.end());
				moves.erase(it);
			}
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

	void Board::CalcMaterialScores()
	{
		materialScore[0] = materialScore[1] = 0;
		for (uint32_t i = 1; i < 13; i ++)
			materialScore[i % 2] += scores[i] * pieces[i].count;
	}

	int32_t Board::Evaluate(Color side) const
	{
		int32_t score = materialScore[White] - materialScore[Black];
		for (uint32_t i = 1; i < 9; i ++)
		{
			for (uint32_t p = 0; p < pieces[i].count; p++)
			{
				if (GetColor(i) == White)
					score += positionWeights[i][pieces[i].positions[p]];
				else
					score -= positionWeights[i][pieces[i].positions[p]];

			}
		}
		return side == White ? -score : score;
	}

	int32_t Board::MVV_LVA(Move move)
	{
		if (pv_table.count(hashKey))
		{
			if (pv_table.at(hashKey) == move)
				return 2000000;
		}
		int32_t score = 0;
		if (!move.Capture())
		{
			if (move == searchKillers[0][ply])
				return 900000;
			else if (move == searchKillers[1][ply])
				return 800000;
			return searchHistory[move.From()][move.To()];
		}

		auto attacker = tiles[move.From()];
		auto victim = move.EnPassant() ? tiles[EnPassantToPiece(move.To())] : tiles[move.To()];
		score += ::chs::MVV_LVA(victim, attacker);

		return score + 1000000;
	}

	int32_t Board::GetPvLine(int32_t depth)
	{
		ET_DEBUG_ASSERT(depth < MAX_DEPTH);
		Move move;
		int32_t count = 0;
		while (count < depth && pv_table.count(hashKey))
		{
			move = pv_table[hashKey];
			if (MovePiece(move))
				pv_moves[count++] = move;
			else
				break;
		}

		for (int32_t i = 0; i < count; i++)
			Revert();

		return count;
	}

	void Board::PrintPvLine(int32_t count) const
	{
		ET_LOG_INFO("PV");
		for (int32_t i = 0; i < count; i++)
			ET_LOG_INFO("\t{}", (Move)pv_moves[i]);
	}

	void Board::ResetForSearch()
	{
		pv_table.clear();
		pv_moves.fill(Move());
		memset(searchKillers[0], 0, sizeof(searchKillers[0]));
		memset(searchKillers[1], 0, sizeof(searchKillers[1]));
		for (int32_t i = 0; i < 64; i++)
			memset(searchHistory[i], 0, sizeof(searchHistory[i]));
		ply = 0;
	}

	void Board::PickNextMove(int32_t index, MoveList& moves)
	{
		ET_DEBUG_ASSERT(index < moves.size());
		Move move = moves[index];
		int32_t bestScore = MVV_LVA(move);
		int32_t bestI = index;

		for (size_t i = index; i < moves.size(); i++)
		{
			int32_t score = MVV_LVA(moves[i]);
			if (score > bestScore)
			{
				bestScore = score;
				bestI = (int32_t)i;
			}
		}

		std::swap(moves[index], moves[bestI]);
	}

#define CHECK_TIME if ((info.nodes & 2047) == 0)\
					{\
						if (et::Time::GetTime() > info.end_time && info.timed)\
							info.stopped = true;\
					}

	int32_t Board::Quiescence(int32_t alpha, int32_t beta, SearchInfo& info)
	{
		ET_DEBUG_ASSERT(Valid());
		info.nodes++;

		CHECK_TIME;

		if (IsRepeated() || fiftyMove >= 100)
			return 0;

		int32_t score = Evaluate(turn);
		if (ply >= MAX_DEPTH)
			return score;

		if (score >= beta)
			return beta;

		if (score > alpha)
			alpha = score;

		MoveList mvs;
		GetAllCaptureMoves(mvs, turn);

		int32_t legal = 0;
		Move bestMove;
		int32_t old_alpha = alpha;
		score = -INF;

		for (size_t i = 0; i < mvs.size(); i++)
		{
			PickNextMove((int32_t)i, mvs);
			auto move = mvs[i];
			if (!MovePiece(move))
				continue;

			legal++;
			score = -Quiescence(-beta, -alpha, info);
			Revert();

			if (info.stopped)
				return 0;

			if (score > alpha)
			{
				if (score >= beta)
				{
					if (legal == 1)
						info.fhf++;
					info.fh++;
					return beta;
				}
				alpha = score;
				bestMove = move;
			}
		}

		if (alpha != old_alpha)
			pv_table[hashKey] = bestMove;

		return alpha;
	}

	int32_t Board::AlphaBeta(int32_t alpha, int32_t beta, int32_t depth, SearchInfo& info)
	{
		ET_DEBUG_ASSERT(Valid());
		info.nodes++;
		if (depth <= 0)
			return Quiescence(alpha, beta, info);

		CHECK_TIME;

		if (IsRepeated() || fiftyMove >= 100)
			return 0;

		if (ply >= MAX_DEPTH)
			return Evaluate(turn);

		MoveList mvs;
		GetAllLegalMoves(mvs, turn);

		int32_t legal = 0;
		Move bestMove;
		int32_t old_alpha = alpha;
		int32_t score = -INF;

		for (size_t i = 0; i < mvs.size(); i++)
		{
			PickNextMove((int32_t)i, mvs);
			auto move = mvs[i];
			if (!MovePiece(move))
				continue;

			legal++;
			score = -AlphaBeta(-beta, -alpha, depth - 1, info);
			Revert();

			if (info.stopped)
				return 0;

			if (score > alpha)
			{
				if (score >= beta)
				{
					if (legal == 1)
						info.fhf++;
					info.fh++;
					if (!move.Capture())
					{
						searchKillers[1][ply] = searchKillers[0][ply];
						searchKillers[0][ply] = move;
					}
					return beta;
				}
				alpha = score;
				bestMove = move;

				if (!move.Capture())
					searchHistory[move.From()][move.To()] += depth;
			}
		}

		if (!legal)
		{
			if (IsAttacked(pieces[BlackKing + turn].positions[0], GetOppColor(turn)))
				return -CHECKMATESCORE + ply;

			return 0;
		}

		if (alpha != old_alpha)
			pv_table[hashKey] = bestMove;

		return alpha;

	}

	Move Board::Search(int32_t depth)
	{
		ET_DEBUG_ASSERT(depth < MAX_DEPTH);
		et::Timer t;
		Move bestMove;
		int32_t bestScore = -INF;

		ResetForSearch();
		SearchInfo info(0.5f * specs.difficulty);

		for (int32_t c_depth = 1; c_depth <= depth; c_depth++)
		{
			bestScore = AlphaBeta(-INF, INF, c_depth, info);

			if (info.stopped)
				break;

			auto i = GetPvLine(c_depth);
			bestMove = pv_moves[0];
			ET_LOG_INFO("Depth: {} Nodes: {} Score: {} Ordering: {} Move: {}", c_depth, info.nodes, bestScore, info.fhf / info.fh, (Move)bestMove);
			PrintPvLine(i);
			/*if (t.Elapsed() >= SEARCH_TIMEOUT)
				break;*/
		}
		ET_LOG_INFO("Time: {}s", t.Elapsed());
		return bestMove;
	}

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

	template <>
	std::ostream& operator<<<std::ostream>(std::ostream& stream, const Board& board)
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