#pragma once
#include <vulkan/vulkan.h>
#include "../texture.h"

namespace et
{
	class Buffer;

	class VulkanImageView : public ImageView
	{
	public:
		VulkanImageView(VkImageView view) : imageView(view) {}
		operator VkImageView() const { return imageView; }
		VkImageView imageView;
	};

	class Image
	{
	public:
		Image(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkSampleCountFlagBits numSamples, VkImageTiling tiling, VkImageUsageFlags imageUsage, VkMemoryPropertyFlags memoryFlags, uint32_t arraylayers = 1, VkImageCreateFlags createflags = 0, VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D,  VkImageAspectFlags imageAspect = VK_IMAGE_ASPECT_COLOR_BIT);
		Image(const Image&) = delete;
		~Image();

		void SetImageData(const VulkanBuffer& buffer);
		void TransitionLayout(VkImageLayout oldLayout, VkImageLayout newLayout);
		void GenerateMipmaps();
		VkImageView GetImageView() const { return imageView; }

		VkImage image;
		VkDeviceMemory memory;
		VkFormat format;
		VkImageView imageView;
		uint32_t width;
		uint32_t height;
		uint32_t mipLevels;
	};

	class VulkanTexture : public Texture
	{
	public:
		VulkanTexture(std::string_view fileName, uint32_t mipLevels = 1);
		VulkanTexture(const std::string& fileName, const TextureCreateInfo& createInfo);
		VulkanTexture(const std::string& textureName, uint32_t width, uint32_t height, const TextureCreateInfo& createInfo);
		VulkanTexture(const std::string& textureName, uint32_t width, uint32_t height, const void* data, const TextureCreateInfo& createInfo);
		~VulkanTexture();
		
		virtual void SetData(void* data) override;

		virtual uint32_t Width() const override { return mpImage->width; }
		virtual uint32_t Height() const override { return mpImage->height; }
		virtual uint32_t Channels() const override { return mChannels; }
		virtual uint32_t MipLevels() const override { return mpImage->mipLevels; }
		virtual TextureFormat Format() const override { return (TextureFormat)mpImage->format; }

		virtual void* GetImGuiTextureID() override;
		virtual void GetTextureData(Ref<Buffer>& buffer) override;
		const Image& GetImage() const { return *mpImage; }
		Ref<ImageView> GetImageView() const { return CreateRef<VulkanImageView>(mpImage->imageView); }
		VkSampler& GetSampler() { return mSampler; }

		operator VkImage() const { return mpImage->image; }
		operator VkImageView() const { return mpImage->imageView; };

		void InitSampler(VkFilter minFilter, VkFilter magFilter, VkSamplerAddressMode addressMode);

		uint32_t mChannels = 0;
		Image* mpImage;
		VkSampler mSampler;
		VkDescriptorSet mImGuiDescriptorSet = VK_NULL_HANDLE;
	};

	class VulkanDepthTexture : public Texture
	{
	public:
		VulkanDepthTexture(uint32_t width, uint32_t height);
		VulkanDepthTexture(uint32_t width, uint32_t height, const TextureCreateInfo& createInfo);
		~VulkanDepthTexture();

		virtual void SetData(void* data) override {}

		virtual uint32_t Width() const override { return mpImage->width; }
		virtual uint32_t Height() const override { return mpImage->height; }
		virtual uint32_t Channels() const override { return 1; }
		virtual uint32_t MipLevels() const override { return 1; }
		virtual TextureFormat Format() const override { return TextureFormat::D24UnormS8Uint; }

		virtual void* GetImGuiTextureID() override { return nullptr; }
		virtual void GetTextureData(Ref<Buffer>& buffer) override { }
		const Image& GetImage() const { return *mpImage; }
		Ref<ImageView> GetImageView() const { return CreateRef<VulkanImageView>(mpImage->imageView); }

	private:
		Image* mpImage;
		VkDescriptorSet mImGuiDescriptorSet = VK_NULL_HANDLE;
	};

	class VulkanFramebuffer : public Framebuffer
	{
	public:
		VulkanFramebuffer(const std::vector<VkImageView>& attachments, uint32_t width, uint32_t height, VkRenderPass render_pass);
		VulkanFramebuffer(const VulkanFramebuffer&) = delete;
		~VulkanFramebuffer();

		operator VkFramebuffer() const { return mFramebuffer; }

	private:
		VkFramebuffer mFramebuffer;
	};
}