#include "tile_manager.h"
#include "Entropy/Entropy.h"

namespace chs
{
	void TileManager::Load(std::string_view fen_string)
	{
		glm::vec2 position(1.f, 1.f);
		for (size_t i = 0; i < fen_string.size(); i++)
		{
			if (std::isalpha(fen_string[i]))
			{
				char lower = std::tolower(fen_string[i]);
				PieceType type = CharToPiece[lower];
				switch (type)
				{
				case PieceType_Pawn:
					pieces.push_back(et::CreateRef<Pawn>(CharToPiece[fen_string[i]]));
					break;
				case PieceType_Rook:
					pieces.push_back(et::CreateRef<Rook>(CharToPiece[fen_string[i]]));
					break;
				case PieceType_Knight:
					pieces.push_back(et::CreateRef<Knight>(CharToPiece[fen_string[i]]));
					break;
				case PieceType_Bishop:
					pieces.push_back(et::CreateRef<Bishop>(CharToPiece[fen_string[i]]));
					break;
				case PieceType_Queen:
					pieces.push_back(et::CreateRef<Queen>(CharToPiece[fen_string[i]]));
					break;
				case PieceType_King:
					pieces.push_back(et::CreateRef<King>(CharToPiece[fen_string[i]]));
					break;
				}
				pieces.back()->position = position;

				position.x++;
			}
			else if (fen_string[i] == ' ')
				break;
			else
			{
				if (std::isdigit(fen_string[i]))
					position.x += (float)(fen_string[i] - '0');
				else
				{
					position.x = 1.f;
					position.y++;
				}
			}
		}
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
					et::Renderer::DrawQuad(q);
				}
			}
		}

		// pieces
		{
			q.color = glm::vec3(1.f, 0.f, 1.f);
			for (auto& piece : pieces)
			{
				q.position = piece->position - glm::vec2(4.5f);
				int32_t texture_index = piece->type;

				et::Renderer::DrawQuad(q, texture_index);
			}
		}
	}

	void TileManager::SetCamera(uint32_t viewportWidth, uint32_t viewportHeight)
	{
		float aspectRatio = (float)viewportWidth / (float)viewportHeight;
		float width = 10.f, height = 10.f;

		if (aspectRatio >= 1.f)
			width = height * aspectRatio;
		else
			height = width / aspectRatio;

		camera.Reset(width, height, 2.f);
	}
}