#include "board.h"

namespace chs
{
	Board::Board(std::string_view fen_string)
	{
		LoadFromFen(fen_string);
	}

	void Board::LoadFromFen(std::string_view fen_string)
	{
		size_t i = 0;

		// board pieces
		int32_t pieceIndex = 0;
		while (fen_string[i] != ' ' && fen_string[i] != 0)
		{
			char c = fen_string[i];
			if (std::isalpha(c))
			{
				tiles[pieceIndex] = CharToPiece[c];
				if (tiles[pieceIndex])
				{
					// get piece list of current piece
					auto& piece = pieces[tiles[pieceIndex]];
					// set position of last piece in piece list and increment piece count
					piece.positions[piece.count++] = pieceIndex;
				}

				pieceIndex++;
			}
			else
			{
				if (std::isdigit(c))
					pieceIndex += c - '0';
			}
			i++;
		}

		ET_DEBUG_ASSERT(fen_string[i] == ' ' || fen_string[i] == 0);
		if (fen_string[i] == 0)
			return;

		turn = (fen_string[++i] == 'w') ? White : Black;

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
			i++;
		}
		else
			i += 2;

		if (fen_string[i] != '-')
		{
			enPassant = FileRankToIndex(fen_string[i], fen_string[i + 1]);
			i += 3;
		}
		else
			i += 2;

		// 50 moves and full moves
	}

	bool Board::Validate()
	{
		return false;
	}

	std::vector<glm::vec2> Board::GetMoveTiles(const glm::vec2& position)
	{
		return {};
	}

	std::vector<uint8_t> Board::GetMoveTiles(uint8_t position)
	{
		return {};
	}

	bool Board::Move(const glm::vec2& from, const glm::vec2& to)
	{
		return false;
	}
}