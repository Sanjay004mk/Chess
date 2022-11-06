#pragma once
#include "Entropy/EntropyUtils.h"
#include "render_info.h"

namespace et
{
	class Renderpass;
	class Buffer;

	class ImageView
	{
	public:
		ImageView() = default;
		virtual ~ImageView() {}
	};

	enum class TextureType
	{
		TwoDimensional = 0,
		Cubemap
	};

	struct TextureCreateInfo
	{
		TextureType type = TextureType::TwoDimensional;
		TextureFormat format = TextureFormat::Undefined;
		TextureUsageFlags usageFlags = 0;
		FilterType minFilter = FilterType::Linear;
		FilterType magFilter = FilterType::Linear;
		SamplerAddressMode addressMode = SamplerAddressMode::Repeat;
		SampleCount sampleCount = SampleCount::Num1;
		MemoryPropertyFlags memoryProperty = MemoryPropertyFlags_DeviceLocal;
		uint32_t mipLevels = 1;
	};

	class Texture
	{
	public:
		Texture() = default;
		Texture(const std::string& name) : name(name) {}
		virtual ~Texture() {}

		virtual void SetData(void* data) = 0;

		virtual uint32_t Width() const = 0;
		virtual uint32_t Height() const = 0;
		virtual uint32_t Channels() const = 0;
		virtual uint32_t MipLevels() const = 0;
		virtual TextureFormat Format() const = 0;
		virtual Ref<ImageView> GetImageView() const = 0;

		virtual void* GetImGuiTextureID() = 0;

		virtual void GetTextureData(Ref<Buffer>& buffer) = 0;

		static TextureCreateInfo depthTextureCreateInfo;

		std::string name{};
	};

	struct FramebufferCreateInfo
	{
		uint32_t width = 0, height = 0;
		std::vector<Ref<ImageView>> attachments;
		Ref<Renderpass> renderpass;
	};

	class Framebuffer
	{
	public:
		Framebuffer() = default;
		Framebuffer(const Framebuffer&) = delete;
		virtual ~Framebuffer() {}

		uint32_t width = 0;
		uint32_t height = 0;
	};

	class FramebufferArray
	{
	public:
		FramebufferArray(const std::vector<FramebufferCreateInfo>& createInfos);
		FramebufferArray(const FramebufferArray&) = delete;
		~FramebufferArray();

		Ref<Framebuffer> operator[](int32_t i) { return (buffers[i]); }
		const Ref<Framebuffer> operator[](int32_t i) const { return (buffers[i]); }

		auto begin() { return buffers.begin(); }
		auto end() { return buffers.end(); }
		auto begin() const { return buffers.begin(); }
		auto end() const { return buffers.end(); }

	private:
		std::vector<Ref<Framebuffer>> buffers;
	};

	Ref<Texture> CreateTexture(const std::string& fileName, const TextureCreateInfo& createInfo);
	Ref<Texture> CreateHDRTexture(const std::string& fileName);

	Ref<Texture> CreateTexture(const std::string& textureName,
		uint32_t width, uint32_t height, 
		const TextureCreateInfo& createInfo);

	Ref<Texture> CreateTexture(const std::string& textureName,
		uint32_t width, uint32_t height,
		const void* data,
		const TextureCreateInfo& createInfo);

	Ref<Texture> CreateDepthTexture(uint32_t width, uint32_t height, const TextureCreateInfo& createInfo = Texture::depthTextureCreateInfo);

	Ref<Framebuffer> CreateFramebuffer(const FramebufferCreateInfo& createInfo);
	Ref<FramebufferArray> CreateFramebufferArray(const std::vector<FramebufferCreateInfo>& createInfos);

}