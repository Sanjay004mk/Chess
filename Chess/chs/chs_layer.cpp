#include <iostream>

#include "chs_layer.h"
#include "Entropy/Entropy.h"

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
					et::ImageLayout::TransferSrc
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
			defaultShader = et::CreateShader("default.shader", createInfo);
		}

		// textures
		{
			textures =
			{
				et::CreateTexture("assets/move_tile.png", et::TextureCreateInfo()),

				et::CreateTexture("assets/pawn.png", et::TextureCreateInfo()),
				et::CreateTexture("assets/pawn_white.png", et::TextureCreateInfo()),

				et::CreateTexture("assets/rook.png", et::TextureCreateInfo()),
				et::CreateTexture("assets/rook_white.png", et::TextureCreateInfo()),

				et::CreateTexture("assets/knight.png", et::TextureCreateInfo()),
				et::CreateTexture("assets/knight_white.png", et::TextureCreateInfo()),

				et::CreateTexture("assets/bishop.png", et::TextureCreateInfo()),
				et::CreateTexture("assets/bishop_white.png", et::TextureCreateInfo())
				,
				et::CreateTexture("assets/queen.png", et::TextureCreateInfo()),
				et::CreateTexture("assets/queen_white.png", et::TextureCreateInfo()),

				et::CreateTexture("assets/king.png", et::TextureCreateInfo()),
				et::CreateTexture("assets/king_white.png", et::TextureCreateInfo()),

			};

			defaultShader->SetTextures("textures", textures);
		}
		// initialize pipeline and framebuffer
		Resize(800, 800);

		// starting positions 
		//tileManager.Load("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
		board = et::CreateRef<Board>("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
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

	bool checkmate = false;

	void ChessLayer::OnImGuiRender()
	{
		// tmp
		if (checkmate)
		{
			ImGui::Begin("Info", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoDocking);// | ImGuiWindowFlags_NoMove);
			ImGui::Text("CHECKMATE");
			ImGui::End();
		}
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
			createInfo.usageFlags = et::TextureUsageFlags_Sampled | et::TextureUsageFlags_ColorAttachment | et::TextureUsageFlags_TransferSrc;
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