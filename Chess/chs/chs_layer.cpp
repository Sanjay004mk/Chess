#include "chs_layer.h"
#include "Entropy/Entropy.h"

namespace chs
{
	void ChessLayer::OnAttach()
	{
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

		{
			et::ShaderCreateInfo createInfo;
			createInfo.uniformDescriptions =
			{
				{
					et::ShaderStage::Vertex,
					0,
					et::ShaderDataType::Mat4f,
					"proj"
				}
			};
			createInfo.vertexAttributes = et::Shader::BasicShaderCreateInfo.vertexAttributes;
			createInfo.vertexBindings = et::Shader::BasicShaderCreateInfo.vertexBindings;
			defaultShader = et::CreateShader("default.shader", createInfo);
		}

		Resize(800, 800);
	}

	void ChessLayer::OnDetach()
	{

	}

	void ChessLayer::OnUpdate(et::TimeStep ts)
	{
		defaultShader->SetUniform("proj", tileManager.GetProjection());
		static et::Quad q;
		q.color = glm::vec3(glm::sin(et::Time::GetTime()), 0.0f, glm::cos(et::Time::GetTime()));
		et::RenderCommand::StartCommandBuffer();
		et::RenderCommand::SetClearColor(glm::vec4(0.1, 0.1, 0.1, 1.0), 0);

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