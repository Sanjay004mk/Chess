#include "tile_manager.h"
#include "Entropy/Entropy.h"

#define MOVE_TILE_TEXTURE_INDEX 0

#if 0

namespace chs
{
	bool InsideBoard(const glm::vec2& pos)
	{
		return pos.x > 0.f && pos.x < 9.f &&
			pos.y > 0.f && pos.y < 9.f;
	}

	void TileManager::Load(std::string_view fen_string)
	{
//		// start from top-left (vulkan -y up system)
//		glm::vec2 position(1.f, 1.f);
//		for (size_t i = 0; i < fen_string.size(); i++)
//		{
//			if (std::isalpha(fen_string[i]))
//			{
//				char lower = std::tolower(fen_string[i]);
//				PieceType type = CharToPiece[lower];
//				switch (type)
//				{
//#define CASE_TYPE(piece) case PieceType_##piece:\
//								pieces[position] = et::CreateRef<piece>(CharToPiece[fen_string[i]]);\
//								break
//
//					CASE_TYPE(Pawn);
//					CASE_TYPE(Rook);
//					CASE_TYPE(Knight);
//					CASE_TYPE(Bishop);
//					CASE_TYPE(Queen);
//					CASE_TYPE(King);
//
//#undef CASE_TYPE
//				}
//
//				position.x++;
//			}
//			else if (fen_string[i] == ' ')
//				break;
//			else
//			{
//				if (std::isdigit(fen_string[i]))
//					position.x += (float)(fen_string[i] - '0');
//
//				// should be '/'
//				else
//				{
//					position.x = 1.f;
//					position.y++;
//				}
//			}
//		}
	}

	void TileManager::DrawTiles()
	{
		et::Quad q;

		// board
		{
			for (float x = -3.5f; x < 4.f; x++)
			{
				for (float y = -3.5f; y < 4.f; y++)
				{
					q.position = { x, y };
					q.color = (int32_t)(x + y) % 2 == 0 ? glm::vec3(0.77, 0.82, 0.69) : glm::vec3(0.19, 0.35, 0.24);

					auto tilePos = q.position + glm::vec2(4.5f);
					if (tilePos == hoveredPiecePos)
						q.color -= glm::vec3(0.1f);

					et::Renderer::DrawQuad(q);
				}
			}
		}

		// highlight moveTiles
		{
			q.color = glm::vec3(1.f);
			for (auto& tile : moveTiles)
			{
				q.position = tile - glm::vec2(4.5f);
				
				et::Renderer::DrawQuad(q, MOVE_TILE_TEXTURE_INDEX);
			}
		}

		// captured pieces
		//{
		//	q.size = glm::vec2(0.5f);
		//	q.color = glm::vec3(1.f);
		//	glm::vec2 blackPos = glm::vec2(4.25f, -3.75f);
		//	glm::vec2 whitePos = glm::vec2(4.25f, 3.75f);
		//	for (auto& piece : capturedPieces)
		//	{
		//		if (piece->IsBlack())
		//		{
		//			q.position = blackPos;
		//			if (blackPos.y > -0.5f)
		//			{
		//				blackPos.y = -3.75f;
		//				blackPos.x = 4.75f;
		//			}
		//			else
		//				blackPos.y += 0.5f;

		//			et::Renderer::DrawQuad(q, piece->type);
		//		}
		//		else
		//		{
		//			q.position = whitePos;
		//			if (whitePos.y < 0.5f)
		//			{
		//				whitePos.y = 3.75f;
		//				whitePos.x = 4.75f;
		//			}
		//			else
		//				whitePos.y -= 0.5f;

		//			et::Renderer::DrawQuad(q, piece->type);
		//		}
		//	}

		//	q.size = glm::vec2(1.f);
		//}

		//// pieces
		//{
		//	q.color = glm::vec3(1.f);
		//	for (auto& [position, piece] : pieces)
		//	{
		//		if (piece == hoveredPiece || piece == clickedPiece)
		//			continue;

		//		q.position = position - glm::vec2(4.5f);
		//		int32_t texture_index = piece->type;

		//		et::Renderer::DrawQuad(q, texture_index);
		//	}
		//	auto drawPiece = [&](const glm::vec2& piecePos, const et::Ref<Piece>& piece, float intensity)
		//	{
		//		q.position = piecePos - glm::vec2(4.5f);
		//		q.color = glm::vec3(intensity);
		//		int32_t texture_index = piece->type;

		//		et::Renderer::DrawQuad(q, texture_index);
		//	};

		//	if (hoveredPiece)
		//		drawPiece(hoveredPiecePos, hoveredPiece, 0.7f);
		//
		//	if (clickedPiece)
		//		drawPiece(clickedPiecePos, clickedPiece, 0.8f);
		//}
	}

	void TileManager::SetCamera(uint32_t viewportWidth, uint32_t viewportHeight)
	{
		screenWidth = (float)viewportWidth;
		screenHeight = (float)viewportHeight;
		float aspectRatio = screenWidth / screenHeight;
		float width = 10.f, height = 10.f;

		if (aspectRatio >= 1.f)
			width = height * aspectRatio;
		else
			height = width / aspectRatio;

		camera.Reset(width, height, 2.f);
	}

	void TileManager::OnUpdate(et::TimeStep ts)
	{
		/*auto mousePos = et::Input::GetMousePosition();

		hoveredPiecePos = ScreenPosToTilePos(mousePos);

		auto it = pieces.find(hoveredPiecePos);
		if (it == pieces.end())
			hoveredPiece.reset();
		else 
			hoveredPiece = it->second;*/
	}

	void TileManager::OnMouseClick(const glm::vec2& mousePos)
	{
		/*clickedPos = ScreenPosToTilePos(mousePos);
		if (!InsideBoard(clickedPos))
			return;

		dragging = true;

		if (!clickedOnce)
		{
			auto it = pieces.find(clickedPos);
			if (it == pieces.end())
				clickedPiece.reset();
			else
			{
				if (!(it->second->IsBlack() ^ blacksTurn))
				{
					clickedPiece = it->second;
					clickedPiecePos = clickedPos;
					GetLegalMoves(clickedPiece, clickedPiecePos, moveTiles);
					clickedOnce = true;
				}
				else
					clickedPiece.reset();
			}
		}
		else
			clickedTwice = true;*/

	}

	void TileManager::OnMouseRelease(const glm::vec2& mousePos)
	{
		//dragging = false;
		//auto pos = ScreenPosToTilePos(mousePos);

		//if (pos == clickedPos)
		//{
		//	if (clickedTwice)
		//	{
		//		auto it = pieces.find(clickedPos);
		//		if (it != pieces.end())
		//		{
		//			if (it->second != clickedPiece && !(it->second->IsBlack() ^ clickedPiece->IsBlack()))
		//			{
		//				moveTiles.clear();
		//				clickedPiece = it->second;
		//				clickedPiecePos = clickedPos;
		//				GetLegalMoves(clickedPiece, clickedPiecePos, moveTiles);
		//				clickedOnce = true;
		//			}
		//			else
		//				goto NO_SELECT;
		//		}
		//		else
		//		{
		//			NO_SELECT:
		//			MovePiece(pos);
		//			clickedOnce = clickedTwice = false;
		//		}
		//	}
		//}
		//else
		//{
		//	// drag drop
		//	MovePiece(pos);
		//	clickedOnce = clickedTwice = false;
		//}
	}

	glm::vec2 TileManager::ScreenPosToTilePos(const glm::vec2& screenPos)
	{
		// convert to (1, 1)[top-left] -> (8, 8)[bottom-right] space
		auto mousePos = screenPos;

		mousePos.x -= screenWidth / 2.f;
		mousePos.x /= (screenWidth / camera.width);
		mousePos.x = std::floor(mousePos.x) + 0.5f;

		mousePos.y -= (screenHeight / 2.f);
		mousePos.y /= (screenHeight / camera.height);
		mousePos.y = std::floor(mousePos.y) + 0.5f;

		mousePos += glm::vec2(4.5f);

		return mousePos;
	}

	//void TileManager::GetLegalMoves(et::Ref<Piece> piece, const glm::vec2& piecePos, std::vector<glm::vec2>& moveTiles)
	//{
		//if (!piece)
		//	return;

		//auto moves = piece->GetVaildMoves();

		//for (auto& move : moves)
		//{
		//	auto tile = piecePos;

		//	while ((move.numTiles > 0 || move.numTiles < 0))
		//	{
		//		tile += move.direction;
		//		if (!InsideBoard(tile))
		//			break;

		//		// if tile contains a piece 
		//		//		1. if selected piece is a pawn break
		//		//		2. if piece in the tile is the same color break
		//		//		3. else add tile to list and break
		//		auto it = pieces.find(tile);
		//		if (it != pieces.end())
		//		{
		//			if (piece->GetType() == PieceType_Pawn)
		//				break;

		//			if (!(piece->IsBlack() ^ it->second->IsBlack()))
		//				break;

		//			moveTiles.push_back(tile);
		//			break;
		//		}

		//		moveTiles.push_back(tile);
		//		move.numTiles--;
		//	}

		//	if (piece->GetType() == PieceType_Pawn)
		//	{
		//		std::vector<glm::vec2> tiles =
		//		{
		//			glm::vec2(1.f, 1.f),
		//			glm::vec2(-1.f, 1.f),
		//		};

		//		if (!piece->IsBlack())
		//			for (auto& tile : tiles)
		//				tile.y = -1.f;

		//		for (auto& tile : tiles)
		//		{
		//			tile += piecePos;
		//			auto it = pieces.find(tile);
		//			bool add = true;
		//			if (it == pieces.end())
		//			{
		//				if (tile != enPassant)
		//					add = false;
		//			}
		//			else
		//			{
		//				if (!(piece->IsBlack() ^ it->second->IsBlack()))
		//					add = false;
		//			}

		//			if (add)
		//				moveTiles.push_back(tile);
		//		}
		//	}
		//	else if (piece->GetType() == PieceType_King)
		//	{
		//		if (!piece->moved)
		//		{
		//			auto kingPos = piecePos;
		//			std::vector<glm::vec2> rooks =
		//			{
		//				glm::vec2(1.f, kingPos.y),
		//				glm::vec2(8.f, kingPos.y)
		//			};
		//			uint32_t i = 0;
		//			for (auto& rook : rooks)
		//			{
		//				bool can_castle = true;
		//				auto it = pieces.find(rook);
		//				// there is a rook in the default position
		//				if (it != pieces.end())
		//				{
		//					// it hasn't moved
		//					if (!it->second->moved)
		//					{
		//						auto CheckPath = [&](float direction)
		//						{
		//							auto pos = kingPos;
		//							for (size_t x = 1; x <= 2; x++)
		//							{
		//								pos += glm::vec2(1.f, 0.f) * direction;
		//								auto piece = pieces.find(pos);
		//								if (piece != pieces.end())
		//									can_castle = false;
		//							}
		//							if (can_castle)
		//							{
		//								moveTiles.push_back(pos);
		//								castles[i] = pos;
		//							}
		//						};

		//						// king side
		//						if (rook.x > 4.f)
		//							CheckPath(1.f);
		//						else // queen side
		//							CheckPath(-1.f);
		//					}
		//				}
		//				i++;
		//			}
		//		}
		//	}
		//	else
		//		castles[0] = castles[1] = glm::vec2(0.f);
		//}
	//}

	void TileManager::MovePiece(const glm::vec2& pos__)
	{
		//auto pos = pos__;

		//if (clickedPiece && InsideBoard(pos))
		//{
		//	auto it = std::find(moveTiles.begin(), moveTiles.end(), pos);
		//	// user moved piece
		//	if (it != moveTiles.end())
		//	{
		//		blacksTurn = !blacksTurn;
		//		auto piece = pieces.find(pos);

		//		if (enPassant == pos && clickedPiece->GetType() == PieceType_Pawn)
		//		{
		//			auto tile = pos;
		//			if (clickedPiece->IsBlack())
		//				tile -= glm::vec2(0.f, 1.f);
		//			else
		//				tile += glm::vec2(0.f, 1.f);

		//			piece = pieces.find(tile);
		//			// piece should exist for enPassant
		//			ET_ASSERT_MSG(piece != pieces.end(), "EM PASSANT NOT WORKING!");
		//			capturedPieces.push_back(piece->second);
		//			pieces.erase(tile);
		//		}

		//		// user captured piece
		//		else if (piece != pieces.end())
		//		{
		//			capturedPieces.push_back(piece->second);
		//			pieces.erase(pos);
		//		}

		//		pieces[pos] = clickedPiece;
		//		pieces.erase(clickedPiecePos);
		//		// clear enPassant after one move
		//		enPassant = glm::vec2(0.f);

		//		// set enPassant
		//		if (clickedPiece->GetType() == PieceType_Pawn && !clickedPiece->moved)
		//		{
		//			if (clickedPiece->IsBlack())
		//				enPassant = pos - glm::vec2(0.f, 1.f);
		//			else
		//				enPassant = pos + glm::vec2(0.f, 1.f);
		//		}
		//		clickedPiece->moved = true;

		//		// castling
		//		{
		//			if (clickedPiece->GetType() == PieceType_King)
		//			{
		//				for (auto& castle : castles)
		//					if (pos == castle)
		//					{
		//						auto rookPos = castle;
		//						// castling to king side
		//						if (castle.x > 4.f)
		//						{
		//							castle.x--;
		//							rookPos.x = 8.f;
		//						}
		//						else // castling to queen side
		//						{
		//							castle.x++;
		//							rookPos.x = 1.f;
		//						}

		//						// castle = new rook position
		//						auto rook = pieces.find(rookPos);
		//						ET_ASSERT_MSG(rook != pieces.end(), "CASTLING NOT WORKING");
		//						// move rook
		//						pieces[castle] = rook->second;
		//						pieces.erase(rookPos);
		//					}
		//			}
		//			castles[0] = castles[1] = glm::vec2(0.f);
		//		}

		//		// chech for checkmate
		//		CheckMate();
		//	}

		//	moveTiles.clear();
		//	clickedPiece.reset();
		//}
	}

	// used in chs_layer.cpp
	extern bool checkmate;

	void TileManager::CheckMate()
	{
		/*bool isBlack = clickedPiece->IsBlack();
		et::Ref<Piece> king = nullptr;
		glm::vec2 kingPos = glm::vec2(0.f);
		for (auto& [pos, piece] : pieces)
		{
			if (isBlack ^ piece->IsBlack())
				if (piece->GetType() == PieceType_King)
				{
					king = piece;
					kingPos = pos; 
					break;
				}
		}

		std::vector<glm::vec2> kingMoveTiles;
		GetLegalMoves(king, kingPos, kingMoveTiles);
		kingMoveTiles.push_back(kingPos);
		for (auto& [pos, piece] : pieces)
		{
			if (piece->IsBlack() == isBlack)
			{
				std::vector<glm::vec2> pieceMoves;
				GetLegalMoves(piece, pos, pieceMoves);
				for (auto& move : pieceMoves)
				{
					auto it = std::find(kingMoveTiles.begin(), kingMoveTiles.end(), move);
					if (it != kingMoveTiles.end())
						kingMoveTiles.erase(it);
				}
			}
		}

		if (kingMoveTiles.empty())
			checkmate = true;*/
	}
}

#else

namespace chs
{
	bool InsideBoard(const glm::vec2& pos)
	{
		return pos.x > 0.f && pos.x < 9.f &&
			pos.y > 0.f && pos.y < 9.f;
	}

	void TileManager::DrawTiles()
	{
		et::Quad q;

		// board
		{
			for (float x = -3.5f; x < 4.f; x++)
			{
				for (float y = -3.5f; y < 4.f; y++)
				{
					q.position = { x, y };
					q.color = (int32_t)(x + y) % 2 == 0 ? glm::vec3(0.77, 0.82, 0.69) : glm::vec3(0.19, 0.35, 0.24);

					auto tilePos = q.position + glm::vec2(4.5f);
					if (tilePos == hoveredPiecePos)
						q.color -= glm::vec3(0.1f);
					if (tilePos == clickedPiecePos)
						q.color -= glm::vec3(0.1f);

					et::Renderer::DrawQuad(q);
				}
			}
		}

		// highlight moveTiles
		{
			q.color = glm::vec3(1.f);
			for (auto& tile : moveTiles)
			{
				q.position = tile - glm::vec2(4.5f);

				et::Renderer::DrawQuad(q, MOVE_TILE_TEXTURE_INDEX);
			}
		}
		
	}

	void TileManager::SetCamera(uint32_t viewportWidth, uint32_t viewportHeight)
	{
		screenWidth = (float)viewportWidth;
		screenHeight = (float)viewportHeight;
		float aspectRatio = screenWidth / screenHeight;
		float width = 10.f, height = 10.f;

		if (aspectRatio >= 1.f)
			width = height * aspectRatio;
		else
			height = width / aspectRatio;

		camera.Reset(width, height, 2.f);
	}

	void TileManager::OnUpdate(et::TimeStep ts)
	{
		hoveredPiecePos = ScreenPosToTilePos(et::Input::GetMousePosition());
	}

	void TileManager::OnMouseClick(const glm::vec2& mousePos)
	{

	}

	void TileManager::OnMouseRelease(const glm::vec2& mousePos)
	{
		clickedPiecePos = ScreenPosToTilePos(mousePos);
	}

	glm::vec2 TileManager::ScreenPosToTilePos(const glm::vec2& screenPos)
	{
		// convert to (1, 1)[top-left] -> (8, 8)[bottom-right] space
		auto mousePos = screenPos;

		mousePos.x -= screenWidth / 2.f;
		mousePos.x /= (screenWidth / camera.width);
		mousePos.x = std::floor(mousePos.x) + 0.5f;

		mousePos.y -= (screenHeight / 2.f);
		mousePos.y /= (screenHeight / camera.height);
		mousePos.y = std::floor(mousePos.y) + 0.5f;

		mousePos += glm::vec2(4.5f);

		return mousePos;
	}

}

#endif