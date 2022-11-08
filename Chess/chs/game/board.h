#pragma once
#include "pieces.h"

namespace chs
{
	class Board
	{
	public:
		Board(std::string_view fen_string);
		~Board() {}

		std::vector<glm::vec2> GetMoveTiles(const glm::vec2& position);
		bool Move(const glm::vec2& from, const glm::vec2& to);
		bool Validate();

	private:
		void LoadFromFen(std::string_view fen_string);
		std::vector<uint8_t> GetMoveTiles(uint8_t position);

		PieceType tiles[64] = { 0 };
		// to be able to index from PieceType enum ( 0 = empty, ... 12 = white king )
		PieceList pieces[13] = {};
		PieceList capturedPieces[13] = {};

		// default white
		Color turn = 0;
		// default no enPassant
		uint8_t enPassant = 64;
		// no castles ( 0000 )
		uint8_t castlePermission = 0;

		template <typename ostream>
		friend ostream& operator<<(ostream& stream, const Board& board);
	};

	template <typename ostream>
	ostream& operator<<(ostream& stream, const Board& board)
	{
		stream << std::endl << "board" << std::endl;
		for (int32_t i = 0; i < 64; i++)
		{
			if (i % 8 == 0)
			{
				stream << std::endl;
				stream << 8 - (i / 8)<< "   ";
			}

			if (board.tiles[i])
				stream << PieceToChar[board.tiles[i]];
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

		return stream;
	}
}