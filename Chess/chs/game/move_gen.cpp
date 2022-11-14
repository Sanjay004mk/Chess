#include <iostream>
#include "Entropy/EntropyUtils.h"

#include "board.h"

namespace chs
{
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
	static void side_capture(const Position& position, int32_t forward, const PieceType tiles[64], int32_t enPassant, bool isWhite, bool prom, int32_t index, MoveList& moves, int32_t side, int32_t& index__)
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

	void PawnMoves(const Board* board, int32_t index, MoveList& moves)
	{
		auto& piece = board->tiles[index];
		ET_DEBUG_ASSERT(IsPawn(piece));

		bool isWhite = IsWhite(piece);
		auto position = Position(index);
		std::array<int32_t, 4> possible = { -1 };

		bool start = isWhite ? (position.position.y == 2) : (position.position.y == 7);
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

	void Slide(const Board* board, int32_t index, const glm::ivec2& direction, MoveList& moves)
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

	void RookMoves(const Board* board, int32_t index, MoveList& moves)
	{
		Slide(board, index, glm::ivec2(1, 0), moves);
		Slide(board, index, glm::ivec2(-1, 0), moves);
		Slide(board, index, glm::ivec2(0, 1), moves);
		Slide(board, index, glm::ivec2(0, -1), moves);
	}

	static void load_knight(const Position& position, const PieceType tiles[64], int32_t index, MoveList& moves, bool isWhite, const glm::ivec2& offs)
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

	void KnightMoves(const Board* board, int32_t index, MoveList& moves)
	{
		Position position(index);

		bool isWhite = IsWhite(board->tiles[index]);

		load_knight(position, board->tiles, index, moves, isWhite, glm::ivec2(1, 2));
		load_knight(position, board->tiles, index, moves, isWhite, glm::ivec2(1, -2));
		load_knight(position, board->tiles, index, moves, isWhite, glm::ivec2(-1, 2));
		load_knight(position, board->tiles, index, moves, isWhite, glm::ivec2(-1, -2));

		load_knight(position, board->tiles, index, moves, isWhite, glm::ivec2(2, -1));
		load_knight(position, board->tiles, index, moves, isWhite, glm::ivec2(2, 1));
		load_knight(position, board->tiles, index, moves, isWhite, glm::ivec2(-2, -1));
		load_knight(position, board->tiles, index, moves, isWhite, glm::ivec2(-2, 1));
	}

	void BishopMoves(const Board* board, int32_t index, MoveList& moves)
	{
		Slide(board, index, glm::ivec2(1, 1), moves);
		Slide(board, index, glm::ivec2(-1, 1), moves);
		Slide(board, index, glm::ivec2(-1, -1), moves);
		Slide(board, index, glm::ivec2(1, -1), moves);
	}

	void QueenMoves(const Board* board, int32_t index, MoveList& moves)
	{
		Slide(board, index, glm::ivec2(1, 0), moves);
		Slide(board, index, glm::ivec2(-1, 0), moves);
		Slide(board, index, glm::ivec2(0, 1), moves);
		Slide(board, index, glm::ivec2(0, -1), moves);
		Slide(board, index, glm::ivec2(1, 1), moves);
		Slide(board, index, glm::ivec2(-1, 1), moves);
		Slide(board, index, glm::ivec2(-1, -1), moves);
		Slide(board, index, glm::ivec2(1, -1), moves);
	}

	static void load_king(const Position& position, const PieceType tiles[64], int32_t index, MoveList& moves, bool isWhite, const glm::ivec2& offs)
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

	static void check_castle(const Board* board, int32_t castlePermission, const PieceType tiles[64], int32_t index, MoveList& moves, bool queenside)
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

	void KingMoves(const Board* board, int32_t index, MoveList& moves)
	{
		Position position(index);


		bool isWhite = IsWhite(board->tiles[index]);

		load_king(position, board->tiles, index, moves, isWhite, glm::ivec2(1, 0));
		load_king(position, board->tiles, index, moves, isWhite, glm::ivec2(-1, 0));
		load_king(position, board->tiles, index, moves, isWhite, glm::ivec2(0, 1));
		load_king(position, board->tiles, index, moves, isWhite, glm::ivec2(0, -1));

		load_king(position, board->tiles, index, moves, isWhite, glm::ivec2(1, -1));
		load_king(position, board->tiles, index, moves, isWhite, glm::ivec2(1, 1));
		load_king(position, board->tiles, index, moves, isWhite, glm::ivec2(-1, -1));
		load_king(position, board->tiles, index, moves, isWhite, glm::ivec2(-1, 1));

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
				glm::ivec2(1,  2),
				glm::ivec2(-1,  2),
				glm::ivec2(1, -2),
				glm::ivec2(-1, -2),

				glm::ivec2(2,  1),
				glm::ivec2(-2,  1),
				glm::ivec2(2, -1),
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

		check(glm::ivec2(1, 0), WhiteRook);
		check(glm::ivec2(-1, 0), WhiteRook);
		check(glm::ivec2(0, 1), WhiteRook);
		check(glm::ivec2(0, -1), WhiteRook);

		check(glm::ivec2(1, 1), WhiteBishop);
		check(glm::ivec2(1, -1), WhiteBishop);
		check(glm::ivec2(-1, 1), WhiteBishop);
		check(glm::ivec2(-1, -1), WhiteBishop);

#undef check

		return false;
	}
}