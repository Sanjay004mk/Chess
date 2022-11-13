#include "move.h"

namespace chs
{
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

	std::ostream& operator<<(std::ostream& stream, const Move& move)
	{
		stream << "Move: [ " << "from: " << IndexToStr(move.From()) << ", to: " << IndexToStr(move.To()) << ", en passant: " << move.EnPassant();
		stream << ", castle: " << move.Castle() << ", capture: " << move.Capture();
		move.Valid() ? stream << " (Valid)" : stream << " (Invalid)";
		stream << " ]";
		return stream;
	}
}