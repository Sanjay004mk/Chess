#include "Entropy/EntropyUtils.h"

#include "board.h"

namespace chs
{
	GetMoveFn GetMoves[13] =
	{
		nullptr,

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
				tiles[pieceIndex] = CharToPiece[c];
				if (tiles[pieceIndex])
				{
					// get piece list of current piece
					auto& piece = pieces[tiles[pieceIndex]];
					// set position of last piece in piece list and increment piece count
					piece.positions[piece.count++] = pieceIndex;
				}
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
	}

	bool Board::Valid()
	{
		return false;
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
			{
				SetBit(majorBitBoard[c], 1, index);
			}
			else if (IsMinor[piece])
			{
				SetBit(minorBitBoard[c], 1, index);
			}
			else
			{
				SetBit(pawnBitBoard[c], 1, index);
			}
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
		if (pos != (pieces[p].count - 1))
		{
			for (size_t i = pos; i < pieces[p].count; i++)
				pieces[p].positions[i] = pieces[p].positions[i + 1];
		}
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
		if (!InsideBoard(position))
			return {};

		auto moves = GetMoveTiles(GetIndexFromPosition(position));
		std::unordered_map<glm::vec2, Move> tiles;
		for (auto& move : moves)
		{
			if (!move.PromotedTo())
				tiles[(GetPositionFromIndex(move.To()))] = move;
		}

		return tiles;
	}

	std::vector<Move> Board::GetMoveTiles(int32_t position)
	{
		auto fn = GetMoves[tiles[position]];
		if (fn)
			return fn(this, position);

		return {};
	}

	bool Board::MovePiece(Move move)
	{
		if (!move.Valid())
			return false;

		if (move.Capture())
		{
			auto cap_tile = move.EnPassant() ? EnPassantToPiece(enPassant) : move.To();
			PieceType rem_piece = 0;
			if (!RemovePiece(cap_tile, &rem_piece))
				return false;

			capturedTiles.push_back({ cap_tile, rem_piece });
		}

		if (move.PawnStart())
			enPassant = PieceToEnPassant(move.To());
		else
			enPassant = 64;

		return ShiftPiece(move.From(), move.To());
	}

	PieceType Board::GetTile(const glm::vec2& tile)
	{
		return GetTile(GetIndexFromPosition(tile));
	}

	bool Board::IsPiece(const glm::vec2& tile)
	{
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
		seed ^= other + 0x9e3779b9 + (seed << 6) + (seed >> 2);;
	}

	size_t Board::hash() const
	{
		size_t seed = 0;
		auto hasher = std::hash<size_t>();
		for (int32_t i = 0; i < 64; i++)
			if (tiles[i])
				hash_combine(seed, hasher(tiles[i] + i));
			
		hash_combine(seed, hasher(enPassant));
		hash_combine(seed, hasher(castlePermission));
		hash_combine(seed, hasher(turn));

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

	std::vector<Move> PawnMoves(const Board* board, int32_t index)
	{
		auto& piece = board->tiles[index];
		ET_DEBUG_ASSERT(IsPawn(piece));

		bool isWhite = IsWhite(piece);

		std::vector<Move> moves;
		auto position = Position(index);
		std::array<int32_t, 4> possible = { -1 };

		bool start =  isWhite ? (position.position.y == 2) : (position.position.y == 7);
		bool prom = isWhite ? (position.position.y == 7) : (position.position.y == 2);
		int32_t capture = 0;

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
					moves.push_back(Move(index, possible[0], capture, 0, 0, 0, BlackRook + isWhite));
					moves.push_back(Move(index, possible[0], capture, 0, 0, 0, BlackKnight + isWhite));
					moves.push_back(Move(index, possible[0], capture, 0, 0, 0, BlackBishop + isWhite));
					moves.push_back(Move(index, possible[0], capture, 0, 0, 0, BlackQueen + isWhite));
					moves.push_back(Move(index, possible[0], capture, 0, 0, 0, 0));
				}
				else
					moves.push_back(Move(index, possible[0], capture, 0, 0, 0, 0));
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
				moves.push_back(Move(index, possible[1], capture, 1, 0, 0, 0));
		}

		// side capture / en passant capture
		auto side_capture = [&](int32_t side, int32_t& index__)
		{
			int32_t enPassant = 0;
			if (position.Move(side, forward, index__))
			{
				bool valid = true;
				if (board->tiles[index__])
				{
					if (IsWhite(board->tiles[index__]) ^ isWhite)
						capture = 1;
					else
						valid = false;
				}
				else if (board->enPassant == index__)
				{
					if (IsWhite(board->tiles[EnPassantToPiece(board->enPassant)]) == isWhite)
						valid = false;

					enPassant = 1;
					capture = 1;
				}
				else
					valid = false;

				if (valid)
				{
					if (prom)
					{
						moves.push_back(Move(index, index__, capture, 0, 0, 0, BlackRook + isWhite));
						moves.push_back(Move(index, index__, capture, 0, 0, 0, BlackKnight + isWhite));
						moves.push_back(Move(index, index__, capture, 0, 0, 0, BlackBishop + isWhite));
						moves.push_back(Move(index, index__, capture, 0, 0, 0, BlackQueen + isWhite));
						moves.push_back(Move(index, index__, capture, 0, 0, 0, 0));
					}
					else
						moves.push_back(Move(index, index__, capture, 0, enPassant, 0, 0));
				}

			}
		};

		side_capture(1, possible[2]);
		side_capture(-1, possible[3]);	

		return moves;
	}

	std::vector<Move> Slide(const Board* board, int32_t index, const glm::ivec2& direction)
	{
		auto start_index = index;
		Position position(index);
		bool isWhite = IsWhite(board->tiles[index]);
		std::vector<Move> moves;

		while (position.Move(direction.x, direction.y, index))
		{
			position.position += direction;

			int32_t capture = 0;
			if (board->tiles[index])
			{
				capture = 1;
				if (!(IsWhite(board->tiles[index]) ^ isWhite))
					break;
			}
			moves.push_back(Move(start_index, index, capture));

			if (capture)
				break;
		}

		return moves;
	}

	static void load(const Board* board, int32_t index, std::vector<Move>& moves, const glm::ivec2& direction)
	{
		auto temp = Slide(board, index, direction);
		if (!temp.empty())
			moves.insert(moves.end(), temp.begin(), temp.end());
	}

	std::vector<Move> RookMoves(const Board* board, int32_t index)
	{
		std::vector<Move> moves;

		load(board, index, moves, glm::ivec2(1, 0));
		load(board, index, moves, glm::ivec2(-1, 0));
		load(board, index, moves, glm::ivec2(0, 1));
		load(board, index, moves, glm::ivec2(0, -1));

		return moves;
	}


	std::vector<Move> KnightMoves(const Board* board, int32_t index)
	{
		std::vector<Move> moves;
		Position position(index);
		auto load_knight = [&](bool isWhite, const glm::ivec2& offs)
		{
			int32_t end_index = -1;
			if (position.Move(offs.x, offs.y, end_index))
			{
				int32_t capture = 0;
				if (board->tiles[end_index])
				{
					capture = 1;
					if (!(IsWhite(board->tiles[end_index]) ^ isWhite))
						return;
				}
				moves.push_back(Move(index, end_index, capture));
			}
		};

		bool isWhite = IsWhite(board->tiles[index]);

		load_knight(isWhite, glm::ivec2( 1,  2));
		load_knight(isWhite, glm::ivec2( 1, -2));
		load_knight(isWhite, glm::ivec2(-1,  2));
		load_knight(isWhite, glm::ivec2(-1, -2));

		load_knight(isWhite, glm::ivec2( 2, -1));
		load_knight(isWhite, glm::ivec2( 2,  1));
		load_knight(isWhite, glm::ivec2(-2, -1));
		load_knight(isWhite, glm::ivec2(-2,  1));

		return moves;
	}

	std::vector<Move> BishopMoves(const Board* board, int32_t index)
	{
		std::vector<Move> moves;

		load(board, index, moves, glm::ivec2(1, 1));
		load(board, index, moves, glm::ivec2(-1, 1));
		load(board, index, moves, glm::ivec2(-1,-1));
		load(board, index, moves, glm::ivec2(1, -1));

		return moves;
	}

	std::vector<Move> QueenMoves(const Board* board, int32_t index)
	{
		std::vector<Move> moves;

		load(board, index, moves, glm::ivec2(1, 0));
		load(board, index, moves, glm::ivec2(-1, 0));
		load(board, index, moves, glm::ivec2(0, 1));
		load(board, index, moves, glm::ivec2(0, -1));
		load(board, index, moves, glm::ivec2(1, 1));
		load(board, index, moves, glm::ivec2(-1, 1));
		load(board, index, moves, glm::ivec2(-1, -1));
		load(board, index, moves, glm::ivec2(1, -1));

		return moves;
	}

	std::vector<Move> KingMoves(const Board* board, int32_t index)
	{
		std::vector<Move> moves;
		Position position(index);
		auto load_king = [&](bool isWhite, const glm::ivec2& offs)
		{
			int32_t end_index = -1;
			if (position.Move(offs.x, offs.y, end_index))
			{
				int32_t capture = 0;
				if (board->tiles[end_index])
				{
					capture = 1;
					if (!(IsWhite(board->tiles[end_index]) ^ isWhite))
						return;
				}
				moves.push_back(Move(index, end_index, capture));
			}
		};

		bool isWhite = IsWhite(board->tiles[index]);

		load_king(isWhite, glm::ivec2( 1,  0));
		load_king(isWhite, glm::ivec2(-1,  0));
		load_king(isWhite, glm::ivec2( 0,  1));
		load_king(isWhite, glm::ivec2( 0, -1));

		load_king(isWhite, glm::ivec2( 1, -1));
		load_king(isWhite, glm::ivec2( 1,  1));
		load_king(isWhite, glm::ivec2(-1, -1));
		load_king(isWhite, glm::ivec2(-1,  1));

		return moves;
	}

}