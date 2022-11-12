#include <iostream>

#include "chs_layer.h"
#include "Entropy/Entropy.h"

#define CHS_INCLUDE_SHADER_STR
#include "assets/assets.h"

#define CLEAR_COLOR 0.25f, 0.21f, 0.23f

namespace chs
{
	static ImVec2 GetImVec2(const glm::vec2& vector)
	{
		ET_STATIC_ASSERT(sizeof(glm::vec2) == sizeof(ImVec2));
		ImVec2 v;
		memcpy_s(&v, sizeof(v), &vector, sizeof(vector));
		return v;
	}

	void ChessLayer::OnAttach()
	{
		// renderpass
		{
			et::RenderpassCreateInfo createInfo;
			createInfo.attachments =
			{
				{
					et::TextureFormat::B8G8R8A8Unorm,
					et::SampleCount::Num1,
					et::AttachmentLoadOp::Clear,
					et::AttachmentStoreOp::Store,
					et::AttachmentLoadOp::DontCare,
					et::AttachmentStoreOp::DontCare,
					et::ImageLayout::Undefined,
					et::ImageLayout::ShaderReadOnly
				}
			};
			createInfo.subpassDesc =
			{
				{
					{et::Attachment()}, // color attachments
					{}, // depth attachments
					{}, // resolve attachments
					0, // srcSubpass
					(uint32_t)(~0), // dstSubpass
					et::PipelineStage::ColorAttachmentOutput,
					et::PipelineStage::Transfer,
					et::AccessFlags::ColorAttachmentWrite,
					et::AccessFlags::TrasferRead,
					et::PipelineBindPoint::Graphics
				}
			};
			renderpass = et::CreateRenderPass(createInfo);
		}

		// shader
		{
			et::ShaderCreateInfo createInfo;
			createInfo.uniformDescriptions =
			{
				{
					et::ShaderStage::Vertex,
					0,
					et::ShaderDataType::Mat4f,
					"proj"
				},
				{
					et::ShaderStage::Fragment,
					1,
					et::ShaderDataType::ImageSampler | et::ShaderDataType::SetArraySize(13),
					"textures"
				}
			};
			createInfo.vertexAttributes = et::Shader::BasicShaderCreateInfo.vertexAttributes;
			createInfo.vertexBindings = et::Shader::BasicShaderCreateInfo.vertexBindings;
			std::stringstream ss(DATA_SHADER_STR);
			defaultShader = et::CreateShader(ss, createInfo);
		}

		// textures
		auto createInfo = et::TextureCreateInfo();
		createInfo.format = et::TextureFormat::R8G8B8A8Unorm;
		{
			textures =
			{
				et::CreateTexture("move_tile", 128, 128, DATA_MOVE_TILE, createInfo),

				et::CreateTexture("pawn_black", 128, 128, DATA_BLACK_PAWN, createInfo),
				et::CreateTexture("pawn_white", 128, 128, DATA_WHITE_PAWN, createInfo),

				et::CreateTexture("rook_black", 128, 128, DATA_BLACK_ROOK, createInfo),
				et::CreateTexture("rook_white", 128, 128, DATA_WHITE_ROOK, createInfo),

				et::CreateTexture("knight_black", 128, 128, DATA_BLACK_KNIGHT, createInfo),
				et::CreateTexture("knight_white", 128, 128, DATA_WHITE_KNIGHT, createInfo),

				et::CreateTexture("bishop_black", 128, 128, DATA_BLACK_BISHOP, createInfo),
				et::CreateTexture("bishop_white", 128, 128, DATA_WHITE_BISHOP, createInfo)
				,
				et::CreateTexture("queen_black", 128, 128, DATA_BLACK_QUEEN, createInfo),
				et::CreateTexture("queen_white", 128, 128, DATA_WHITE_QUEEN, createInfo),

				et::CreateTexture("king_black", 128, 128, DATA_BLACK_KING, createInfo),
				et::CreateTexture("king_white", 128, 128, DATA_WHITE_KING, createInfo),

			};
			defaultShader->SetTextures("textures", textures);
		}
		// initialize pipeline and framebuffer
		Resize(800, 800);
		PreComputeBoardHashes();	
	}

	void ChessLayer::OnDetach()
	{

	}

	void ChessLayer::StartNewGame()
	{
		StartGame("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
	}

	bool ChessLayer::StartGame(std::string_view fen_string)
	{
		Board temp(fen_string);
		if (!temp.Valid())
			return false;
		board = et::CreateRef<Board>(temp);
		tileManager.board = board.get();
		return true;
	}

	void ChessLayer::OnUpdate(et::TimeStep ts)
	{
		tileManager.OnUpdate(ts);

		defaultShader->SetUniform("proj", tileManager.GetProjection());
		et::RenderCommand::StartCommandBuffer();
		et::RenderCommand::SetClearColor(glm::vec4(CLEAR_COLOR, 1.0f), 0);

		et::Renderer::BeginRenderpass(renderpass, framebuffer);
		et::Renderer::BindPipeline(pipeline);
		et::Renderer::BindShader(defaultShader);

		tileManager.DrawTiles();

		et::Renderer::Flush();
		et::Renderer::EndRenderpass();

		et::RenderCommand::EndCommandBuffer();

		et::Renderer::Present(screen);
	}

	extern bool askPromotion;
	static char fen[256] = {};

	void ChessLayer::DisplayBoardUI()
	{
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoDecoration;
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImVec2 viewport_size = viewport->Size;
		ImVec2 viewport_pos = viewport->Pos;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(CLEAR_COLOR, 1.f));

		float wtos = tileManager.WorldToScreenUnit();

		if (askPromotion)
		{
			auto pos = tileManager.WorldPosToScreenPos(glm::vec2(-3.5f, 0.5f));
			
			// the turn will have changed after board->MoveTile() so invert it
			bool isWhite = !board->GetTurn();
			PieceType piece = BlackRook + isWhite;
			char id[2] = { 0 };
			id[0] = 'a';
			for (size_t i = 0; i < 4; i++)
			{
				ImGui::SetNextWindowPos(ImVec2(viewport_pos.x + pos.x, viewport_pos.y + pos.y));
				ImGui::Begin(id, nullptr, window_flags);
				if (ImGui::ImageButton(textures[piece]->GetImGuiTextureID(), ImVec2(wtos, wtos)))
					tileManager.Promote(piece);
				ImGui::End();
				id[0]++;
				piece += 2;
				pos.x += 2.f * wtos;;
			}
		}

		// file and rank
		{

			{
				glm::vec2 pos = tileManager.WorldPosToScreenPos(glm::vec2(-4.5f, 3.5f));
				pos.x -= ImGui::GetFontSize() / 2.f;
				pos.y -= ImGui::GetFontSize() / 2.f;

				char prstr[2] = { 0 };
				prstr[0] = '8';
				while (prstr[0] >= '1')
				{
					ImGui::SetNextWindowPos(ImVec2(viewport_pos.x + pos.x, viewport_pos.y + pos.y));
					ImGui::Begin(prstr, nullptr, window_flags);
					ImGui::Text("%c", prstr[0]);
					ImGui::End();
					pos.y += wtos;
					prstr[0]--;
				}

				prstr[0] = 'A';
				pos.x += tileManager.WorldToScreenUnit();
				while (prstr[0] <= 'H')
				{
					ImGui::SetNextWindowPos(ImVec2(viewport_pos.x + pos.x, viewport_pos.y + pos.y));
					ImGui::Begin(prstr, nullptr, window_flags);
					ImGui::Text("%c", prstr[0]);
					ImGui::End();
					pos.x += wtos;
					prstr[0]++;
				}
			}
		}


		// full moves and fifty moves
		{
			bool wide = viewport_size.x / viewport_size.y > 1.66f;
			glm::vec2 fullmove_pos = tileManager.WorldPosToScreenPos(glm::vec2(-3.5f, 4.7f));
			glm::vec2 fiftymove_pos = tileManager.WorldPosToScreenPos(glm::vec2(0.5f, 4.7f));
			std::string fulstr{}, fifstr{};
			fulstr = fmt::format("Moves: {}", board->Full());
			fifstr = fmt::format("Fifty Move: {}", board->Fifty());
			if (wide)
			{
				fullmove_pos = tileManager.WorldPosToScreenPos(glm::vec2(5.5f, 0.7f));
				fiftymove_pos = tileManager.WorldPosToScreenPos(glm::vec2(5.5f, -0.3f));
				float extra = (viewport_size.x - tileManager.WorldPosToScreenPos(glm::vec2(5.5f, 0.f)).x) / 2.f;

				fullmove_pos.x += extra - (ImGui::CalcTextSize(fulstr.c_str()).x / 2.f);
				fiftymove_pos.x += extra - (ImGui::CalcTextSize(fifstr.c_str()).x / 2.f);
			}

			ImGui::SetNextWindowPos(ImVec2(viewport_pos.x + fullmove_pos.x, viewport_pos.y + fullmove_pos.y));
			ImGui::Begin(fulstr.c_str(), nullptr, window_flags);
			ImGui::Text("%s", fulstr.c_str());
			ImGui::End();

			ImGui::SetNextWindowPos(ImVec2(viewport_pos.x + fiftymove_pos.x, viewport_pos.y + fiftymove_pos.y));
			ImGui::Begin(fifstr.c_str(), nullptr, window_flags);
			ImGui::Text("%s", fifstr.c_str());
			ImGui::End();
		}

		// display checkmate
		{
			static float padding = 35.f;
			auto [mate, color] = board->InCheckMate();
			if (mate)
			{
				ImVec2 text_size = ImGui::CalcTextSize("Checkmate!");
				text_size.x += padding;
				text_size.y += padding;

				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(padding / 2.f, padding / 2.f));
				ImGui::SetNextWindowPos(ImVec2(viewport_pos.x + (viewport_size.x - text_size.x) / 2.f, viewport_pos.y + (viewport_size.y - text_size.y) / 2.f));
				ImGui::Begin("Checkmate!", nullptr, window_flags);
				ImGui::Text("Checkmate!");
				ImGui::End();
				ImGui::PopStyleVar();
			}
		}

		// home button
		{
			ImGui::SetNextWindowPos(ImVec2(viewport_pos.x + wtos * 0.1f, viewport_pos.y + wtos * 0.1f));
			ImGui::Begin("back_button", nullptr, window_flags);
			if (ImGui::Button("<-", ImVec2(wtos * 0.8f, wtos * 0.8f)))
			{
				board.reset();
				tileManager.board = nullptr;
				memset(fen, 0, sizeof(fen));
			}
			ImGui::End();
		}

		ImGui::PopStyleVar(3);
		ImGui::PopStyleColor();
	}

	void ChessLayer::DisplayMainMenu()
	{
		ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoDecoration;
		window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
		window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

		ImGuiViewport* viewport = ImGui::GetMainViewport();
		ImVec2 viewport_size = viewport->Size;
		ImVec2 viewport_pos = viewport->Pos;

		ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
		ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(CLEAR_COLOR, 1.f));

		float wtos = tileManager.WorldToScreenUnit();

		ImGui::SetNextWindowPos(viewport_pos);
		ImGui::SetNextWindowSize(viewport_size);
		ImGui::Begin("MENU", nullptr, window_flags);
		// mode selection ( vs player / computer )
		if (inMainMenu)
		{
			// tmp
			ImVec2 text_size = ImGui::CalcTextSize("Vs Player");
			text_size.x += wtos;
			text_size.y += wtos;

			ImGui::SetCursorPosY((viewport_size.y - text_size.y) / 2.f);
			ImGui::SetCursorPosX((viewport_size.x - text_size.x) / 2.f);
			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(wtos / 2.f, wtos / 2.f));
			if (ImGui::Button("Vs Player", ImVec2(text_size)))
				inMainMenu = false;

			ImGui::PopStyleVar();
		}
		// start new game / load from fen
		else
		{
			// tmp
			ImVec2 text_size = ImGui::CalcTextSize("Start New Game");
			text_size.x += wtos;
			text_size.y += wtos;

			ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(wtos / 2.f, wtos / 2.f));
			ImGui::SetCursorPosY((viewport_size.y - text_size.y * 4) / 2.f);
			ImGui::SetCursorPosX((viewport_size.x - text_size.x) / 2.f);

			if (ImGui::Button("Start New Game", ImVec2(text_size)))
			{
				StartNewGame();
				inMainMenu = true;
			}

			static float fen_inp_len = 24 * ImGui::GetFontSize();
			ImGui::SetCursorPosX((viewport_size.x - fen_inp_len) / 2.f);
			ImGui::SetCursorPosY((viewport_size.y - text_size.y ) / 2.f);
			ImGui::PushItemWidth(fen_inp_len);
			ImGui::InputText("##fen", fen, 256);

			text_size.x = wtos + ImGui::CalcTextSize("Load From FEN").x;
			ImGui::SetCursorPosY((viewport_size.y + text_size.y * 2.f) / 2.f);
			ImGui::SetCursorPosX((viewport_size.x - text_size.x) / 2.f);
			if (ImGui::Button("Load From FEN", ImVec2(text_size)))
			{
				if (StartGame(fen))
					inMainMenu = true;
			}

			ImGui::PopStyleVar();
		}
		ImGui::End();

		ImGui::PopStyleVar(3);
		ImGui::PopStyleColor();
	}

	void ChessLayer::OnImGuiRender()
	{
		if (board)
			DisplayBoardUI();
		else
			DisplayMainMenu();

#if defined(CHS_DEBUG)
		ImGui::SetNextWindowPos(viewport_pos);
		ImGui::Begin("fps", nullptr, window_flags);
		ImGui::Text("%.4f", ImGui::GetIO().Framerate);
		ImGui::End();
#endif
		
	}

	static bool perft = false;

	void ChessLayer::OnEvent(et::Event& e)
	{
		et::EventDispatcher dispatcher(e);
		dispatcher.Dispatch<et::WindowResizeEvent>([this](et::WindowResizeEvent& e)
			{
				this->Resize(e.GetWidth(), e.GetHeight());
				return false;
			});

		dispatcher.Dispatch<et::MouseButtonPressedEvent>([this](et::MouseButtonPressedEvent& e)
			{
				if (e.GetMouseButton() == et::Mouse::ButtonLeft)
					this->tileManager.OnMouseClick(et::Input::GetMousePosition());
				return false;
			});

		dispatcher.Dispatch<et::MouseButtonReleasedEvent>([this](et::MouseButtonReleasedEvent& e)
			{
				if (e.GetMouseButton() == et::Mouse::ButtonLeft)
					this->tileManager.OnMouseRelease(et::Input::GetMousePosition());
				return false;
			});

		dispatcher.Dispatch<et::KeyPressedEvent>([this](et::KeyPressedEvent& e)
			{
				bool control = et::Input::IsKeyDown(et::Key::LeftControl) || et::Input::IsKeyDown(et::Key::RightControl);
				bool shift = et::Input::IsKeyDown(et::Key::LeftShift) || et::Input::IsKeyDown(et::Key::RightShift);

				if (board)
				{
					if (perft)
					{
						perft = false;
						int32_t depth = e.GetKeyCode() - et::Key::D0;
						if (depth > 0 && depth < 10)
							this->board->PerftRoot(depth);
					}
					else if (control)
					{
						if (e.GetKeyCode() == et::Key::Z)
							this->board->Undo();
						else if (e.GetKeyCode() == et::Key::C)
							ImGui::SetClipboardText(this->board->GetFEN().c_str());
						else if (e.GetKeyCode() == et::Key::P)
						{
							if (shift)
								ET_LOG_INFO("{}", *board);
							else
								ET_LOG_INFO("hash: 0x{:x}", this->board->GetHash());
						}
						else if (e.GetKeyCode() == et::Key::U)
							perft = true;
						else if (e.GetKeyCode() == et::Key::N)
							StartNewGame();
					}
				}

				return false;
			});
	}

	void ChessLayer::Resize(uint32_t newWidth, uint32_t newHeight)
	{
		if (newWidth == width && newHeight == height)
			return;

		width = newWidth;
		height = newHeight;

		tileManager.SetCamera(width, height);

		{
			et::TextureCreateInfo createInfo;
			// swapchain format
			createInfo.format = et::Renderer::GetSurfaceFormat();
			createInfo.usageFlags = et::TextureUsageFlags_Sampled | et::TextureUsageFlags_ColorAttachment;
			screen = et::CreateTexture("screen", width, height, createInfo);
		}

		{
			et::FramebufferCreateInfo createInfo;
			createInfo.attachments = { screen->GetImageView() };
			createInfo.width = width;
			createInfo.height = height;
			createInfo.renderpass = renderpass;
			framebuffer = et::CreateFramebuffer(createInfo);
		}

		{
			et::PipelineCreateInfo createInfo;
			createInfo.renderpass = renderpass;
			createInfo.shader = defaultShader;
			createInfo.colorAttachmentCount = 1;
			createInfo.colorBlendEnable = false;
			createInfo.cullMode = et::CullMode::None;
			createInfo.depthEnable = false;
			createInfo.viewport = { 0.f, 0.f, (float)width, (float)height, 0.f, 1.f };
			createInfo.scissors = { glm::vec2(0.f), glm::vec2((float)width, (float)height) };
			pipeline = et::CreatePipeline(createInfo);
		}
	}
}