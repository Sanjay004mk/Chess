#include <iostream>

#include "chs_layer.h"
#include "Entropy/Entropy.h"

#define CHS_INCLUDE_SHADER_STR
#include "assets/assets.h"

namespace chs
{
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

		// starting positions 
		//board = et::CreateRef<Board>("rnbqkb1r/pp2pppp/7n/1Ppp4/4P3/8/P1PP1PPP/RNBQKBNR w KQkq c6 0 4");
		board = et::CreateRef<Board>("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
		tileManager.board = board.get();
		std::cout << *board << std::endl;
	}

	void ChessLayer::OnDetach()
	{

	}

	void ChessLayer::OnUpdate(et::TimeStep ts)
	{
		tileManager.OnUpdate(ts);

		defaultShader->SetUniform("proj", tileManager.GetProjection());
		et::RenderCommand::StartCommandBuffer();
		et::RenderCommand::SetClearColor(glm::vec4(0.25, 0.21, 0.23, 1.0), 0);

		et::Renderer::BeginRenderpass(renderpass, framebuffer);
		et::Renderer::BindPipeline(pipeline);
		et::Renderer::BindShader(defaultShader);

		tileManager.DrawTiles();

		et::Renderer::Flush();
		et::Renderer::EndRenderpass();

		et::RenderCommand::EndCommandBuffer();

		et::Renderer::Present(screen);
	}

	void ChessLayer::OnImGuiRender()
	{

	}

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
				if (control)
					if (e.GetKeyCode() == et::Key::Z)
						this->board->Undo();
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