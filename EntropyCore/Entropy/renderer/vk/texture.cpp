#include <etpch.h>
#include <ImGui/imgui_impl_vulkan.h>
#include <stb_image/stb_image.h>

#include "vkapi.h"
#include "../renderer.h"

namespace et
{
	TextureCreateInfo Texture::depthTextureCreateInfo =
	{
		TextureType::TwoDimensional,
		TextureFormat::D24UnormS8Uint,
		TextureUsageFlags_DepthStencilAttachment,
		FilterType::Linear,
		FilterType::Linear,
		SamplerAddressMode::Repeat,
		SampleCount::Num1,
		MemoryPropertyFlags_DeviceLocal,
		false
	};

	Image::Image(uint32_t width, uint32_t height, uint32_t mipLevels, VkFormat format, VkSampleCountFlagBits numSamples, VkImageTiling tiling, VkImageUsageFlags imageUsage, VkMemoryPropertyFlags memoryFlags, uint32_t arrayLayers, VkImageCreateFlags createflags, VkImageViewType viewType, VkImageAspectFlags imageAspect)
		: width(width), height(height), mipLevels(mipLevels), format(format)
	{
		VkResult err;

		VkImageCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		info.flags = createflags;
		info.arrayLayers = arrayLayers;
		info.extent.width = width;
		info.extent.height = height;
		info.extent.depth = 1;
		info.format = format;
		info.imageType = VK_IMAGE_TYPE_2D;
		info.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		info.mipLevels = mipLevels;
		info.usage = imageUsage;
		info.tiling = tiling;
		info.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
		info.samples = numSamples;

		err = vkCreateImage(VulkanAPI::GetDevice(), &info, nullptr, &image);
		ET_VK_ASSERT_MSG(err, "Failed to create image!");

		VkMemoryRequirements requirements{};
		vkGetImageMemoryRequirements(VulkanAPI::GetDevice(), image, &requirements);

		VkMemoryAllocateInfo alloc_info{};
		alloc_info.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		alloc_info.allocationSize = requirements.size;
		alloc_info.memoryTypeIndex = VulkanAPI::GetMemoryTypeIndex(requirements.memoryTypeBits, memoryFlags);

		err = vkAllocateMemory(VulkanAPI::GetDevice(), &alloc_info, nullptr, &memory);
		ET_VK_ASSERT_MSG(err, "Failed to allocate image memory!");

		err = vkBindImageMemory(VulkanAPI::GetDevice(), image, memory, 0);
		ET_VK_ASSERT_MSG(err, "Failed to bind image memory!");

		VkImageViewCreateInfo view_info{};
		view_info.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		view_info.format = format;
		view_info.image = image;
		view_info.viewType = viewType;
		view_info.subresourceRange.aspectMask = imageAspect;
		view_info.subresourceRange.baseArrayLayer = 0;
		view_info.subresourceRange.baseMipLevel = 0;
		view_info.subresourceRange.layerCount = arrayLayers;
		view_info.subresourceRange.levelCount = mipLevels;

		err = vkCreateImageView(VulkanAPI::GetDevice(), &view_info, nullptr, &imageView);
		ET_VK_ASSERT_MSG(err, "Failed to create image view!");
	}

	Image::~Image()
	{
		vkDestroyImageView(VulkanAPI::GetDevice(), imageView, nullptr);
		vkDestroyImage(VulkanAPI::GetDevice(), image, nullptr);
		vkFreeMemory(VulkanAPI::GetDevice(), memory, nullptr);
	}

	void Image::SetImageData(const VulkanBuffer& buffer)
	{
		CommandBuffer command_buffer;
		VkBufferImageCopy range{};
		range.imageExtent.width = width;
		range.imageExtent.height = height;
		range.imageExtent.depth = 1;
		range.bufferImageHeight =  range.bufferRowLength = 0;
		range.bufferOffset = 0;
		range.imageOffset = { 0, 0, 0 };
		range.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		range.imageSubresource.baseArrayLayer = 0;
		range.imageSubresource.layerCount = 1;
		range.imageSubresource.mipLevel = 0;

		command_buffer.Begin();
		vkCmdCopyBufferToImage(command_buffer, (VkBuffer)buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, & range);
		command_buffer.End();
		command_buffer.Submit();
	}

	void Image::TransitionLayout(VkImageLayout oldLayout, VkImageLayout newLayout)
	{
		CommandBuffer command_buffer;

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.image = image;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = mipLevels;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		auto setMaskAndStageFlags = [](VkImageLayout layout, VkAccessFlags& accessMask, VkPipelineStageFlags& stageFlags)
		{
			if (layout == VK_IMAGE_LAYOUT_UNDEFINED)
			{
				accessMask = 0;
				stageFlags = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			}
			else if (layout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
			{
				accessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
				stageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
			}
			else if (layout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
			{
				accessMask = VK_ACCESS_SHADER_READ_BIT;
				stageFlags = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
			}
			else if (layout == VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL)
			{
				accessMask = VK_ACCESS_TRANSFER_READ_BIT;
				stageFlags = VK_PIPELINE_STAGE_TRANSFER_BIT;
			}
			else {
				ET_LOG_ERROR("Unhandled case of image layout transition!");
				return;
			}
		};

		setMaskAndStageFlags(oldLayout, barrier.srcAccessMask, sourceStage);
		setMaskAndStageFlags(newLayout, barrier.dstAccessMask, destinationStage);

		command_buffer.Begin();
		vkCmdPipelineBarrier(
			command_buffer,
			sourceStage, destinationStage,
			0,
			0, nullptr,
			0, nullptr,
			1, &barrier
		);
		command_buffer.End();
		command_buffer.Submit();
	}

	void Image::GenerateMipmaps()
	{
		VkFormatProperties formatProperties;
		vkGetPhysicalDeviceFormatProperties(VulkanAPI::GetPhysicalDevice(), format, &formatProperties);
		ET_ASSERT_MSG((formatProperties.optimalTilingFeatures & VK_FORMAT_FEATURE_SAMPLED_IMAGE_FILTER_LINEAR_BIT), "texture image format does not support linear blitting!");

		CommandBuffer command_buffer;

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.image = image;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;
		barrier.subresourceRange.levelCount = 1;

		int32_t mipWidth = width;
		int32_t mipHeight = height;
		command_buffer.Begin();
		for (uint32_t i = 1; i < mipLevels; i++)
		{
			barrier.subresourceRange.baseMipLevel = i - 1;
			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;

			vkCmdPipelineBarrier(
				command_buffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			VkImageBlit blit{};
			blit.srcOffsets[0] = { 0, 0, 0 };
			blit.srcOffsets[1] = { mipWidth, mipHeight, 1 };
			blit.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.srcSubresource.mipLevel = i - 1;
			blit.srcSubresource.baseArrayLayer = 0;
			blit.srcSubresource.layerCount = 1;
			blit.dstOffsets[0] = { 0, 0, 0 };
			blit.dstOffsets[1] = { mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
			blit.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			blit.dstSubresource.mipLevel = i;
			blit.dstSubresource.baseArrayLayer = 0;
			blit.dstSubresource.layerCount = 1;

			vkCmdBlitImage(
				command_buffer,
				image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
				image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1, &blit,
				VK_FILTER_LINEAR);

			barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL;
			barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			vkCmdPipelineBarrier(
				command_buffer,
				VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
				0, nullptr,
				0, nullptr,
				1, &barrier);

			if (mipWidth > 1) mipWidth /= 2;
			if (mipHeight > 1) mipHeight /= 2;
		}

		barrier.subresourceRange.baseMipLevel = mipLevels - 1;
		barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

		vkCmdPipelineBarrier(
			command_buffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0,
			0, nullptr,
			0, nullptr,
			1, &barrier);

		command_buffer.End();
		command_buffer.Submit();
	}

	VulkanTexture::VulkanTexture(const std::string& fileName, const TextureCreateInfo& createInfo)
		: Texture(get_file_name(fileName))
	{
		int32_t width = 0, height = 0;
		uint8_t* data = (uint8_t*)stbi_load(fileName.c_str(), &width, &height, (int32_t*)(&mChannels), STBI_rgb_alpha);
		ET_ASSERT_MSG(data, "Failed to load texture: {0}", fileName);
		size_t size = width * height * 4;
		uint32_t mip_levels = createInfo.mipLevels;

		// if mip_levels is not set
		if (mip_levels < 1)
			mip_levels = (uint32_t)(std::floor(std::log2(std::max(width, height)))) + 1;

		VulkanBuffer staging_buffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		staging_buffer.SetDataMapped(data, size);
		stbi_image_free(data);

		VkFormat format = VK_FORMAT_R8G8B8A8_UNORM;

		VkImageUsageFlags imageUsageFlags = 
			(createInfo.usageFlags != 0) ? 
				(VkImageUsageFlags)createInfo.usageFlags : 
				(VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

		mpImage = new Image((uint32_t)width, (uint32_t)height, mip_levels, format, (VkSampleCountFlagBits)createInfo.sampleCount, VK_IMAGE_TILING_OPTIMAL, imageUsageFlags, (VkMemoryPropertyFlags)createInfo.memoryProperty);
		mpImage->TransitionLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		mpImage->SetImageData(staging_buffer);

		if (mip_levels > 1)
			mpImage->GenerateMipmaps();
		else
			mpImage->TransitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		InitSampler((VkFilter)createInfo.minFilter, (VkFilter)createInfo.magFilter, (VkSamplerAddressMode)createInfo.addressMode);
	}

	VulkanTexture::VulkanTexture(std::string_view fileName, uint32_t mipLevels)
		: Texture(get_file_name(fileName))
	{
		int32_t width = 0, height = 0;
		float* data = (float*)stbi_loadf(fileName.data(), &width, &height, (int32_t*)(&mChannels), STBI_rgb_alpha);
		ET_ASSERT_MSG(data, "Failed to load texture: {0}", fileName);
		size_t size = width * height * 4 * 4;

		uint32_t mip_levels = mipLevels;
		VulkanBuffer staging_buffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		staging_buffer.SetDataMapped(data, size);
		stbi_image_free(data);

		VkFormat format = VK_FORMAT_R32G32B32A32_SFLOAT;

		VkImageUsageFlags imageUsageFlags = (VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

		mpImage = new Image((uint32_t)width, (uint32_t)height, mip_levels, format, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL, imageUsageFlags, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
		mpImage->TransitionLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		mpImage->SetImageData(staging_buffer);
		mpImage->TransitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		InitSampler(VK_FILTER_LINEAR, VK_FILTER_LINEAR, VK_SAMPLER_ADDRESS_MODE_REPEAT);
	}

	VulkanTexture::VulkanTexture(const std::string& textureName, uint32_t width, uint32_t height, const TextureCreateInfo& createInfo)
		: Texture(get_file_name(textureName))
	{
		uint32_t arrayLayers = 1;
		VkImageCreateFlags createFlags = 0;
		VkImageViewType viewType = VK_IMAGE_VIEW_TYPE_2D;
		VkImageAspectFlags aspect = VK_IMAGE_ASPECT_COLOR_BIT;
		if (createInfo.type == TextureType::Cubemap)
		{
			arrayLayers = 6;
			createFlags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;
			viewType = VK_IMAGE_VIEW_TYPE_CUBE;
		}
		if (createInfo.format == TextureFormat::D24UnormS8Uint)
			aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
		mpImage = new Image(width, height, createInfo.mipLevels, (VkFormat)createInfo.format, (VkSampleCountFlagBits)createInfo.sampleCount, VK_IMAGE_TILING_OPTIMAL, (VkImageUsageFlags)createInfo.usageFlags, (VkMemoryPropertyFlags)createInfo.memoryProperty, arrayLayers, createFlags, viewType, aspect);
		InitSampler((VkFilter)createInfo.minFilter, (VkFilter)createInfo.magFilter, (VkSamplerAddressMode)createInfo.addressMode);
	}

	VulkanTexture::VulkanTexture(const std::string& textureName, uint32_t width, uint32_t height, const void* data, const TextureCreateInfo& createInfo)
		: Texture(textureName)
	{
		mChannels = GetNumChannels(createInfo.format);
		size_t size = width * height * mChannels;
		uint32_t mip_levels = createInfo.mipLevels;

		// if mip levels is not set
		if (mip_levels < 1)
			mip_levels = (uint32_t)(std::floor(std::log2(std::max(width, height)))) + 1;

		VulkanBuffer staging_buffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

		staging_buffer.SetDataMapped(data, size);

		VkFormat format = (VkFormat)createInfo.format;

		VkImageUsageFlags imageUsageFlags =
			(createInfo.usageFlags != 0) ?
			(VkImageUsageFlags)createInfo.usageFlags :
			(VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

		mpImage = new Image((uint32_t)width, (uint32_t)height, mip_levels, format, (VkSampleCountFlagBits)createInfo.sampleCount, VK_IMAGE_TILING_OPTIMAL, imageUsageFlags, (VkMemoryPropertyFlags)createInfo.memoryProperty);
		mpImage->TransitionLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		mpImage->SetImageData(staging_buffer);

		if (mip_levels > 1)
			mpImage->GenerateMipmaps();
		else
			mpImage->TransitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);

		InitSampler((VkFilter)createInfo.minFilter, (VkFilter)createInfo.magFilter, (VkSamplerAddressMode)createInfo.addressMode);
	}

	VulkanTexture::~VulkanTexture()
	{
		delete mpImage;
		vkDestroySampler(VulkanAPI::GetDevice(), mSampler, nullptr);
	}

	void VulkanTexture::InitSampler(VkFilter minFilter, VkFilter magFilter, VkSamplerAddressMode addressMode)
	{
		VkPhysicalDeviceProperties properties{};
		vkGetPhysicalDeviceProperties(VulkanAPI::GetPhysicalDevice(), &properties);

		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.magFilter = magFilter;
		samplerInfo.minFilter = minFilter;
		samplerInfo.addressModeU = addressMode;
		samplerInfo.addressModeV = addressMode;
		samplerInfo.addressModeW = addressMode;
		samplerInfo.anisotropyEnable = VK_FALSE;
		samplerInfo.maxAnisotropy = properties.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;
		if (minFilter == magFilter == VK_FILTER_LINEAR)
			samplerInfo.maxLod = (float)(mpImage->mipLevels - 1);
		samplerInfo.mipLodBias = 0.0f;

		VkResult err = vkCreateSampler(VulkanAPI::GetDevice(), &samplerInfo, nullptr, &mSampler);
		ET_VK_ASSERT_MSG(err, "Failed to create sampler!");
	}

	void VulkanTexture::SetData(void* data)
	{
		size_t size = mpImage->width * mpImage->height * mChannels;
		VulkanBuffer staging_buffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
		staging_buffer.SetDataMapped(data, size);

		mpImage->TransitionLayout(VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		mpImage->SetImageData(staging_buffer);
		mpImage->TransitionLayout(VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
	}

	void* VulkanTexture::GetImGuiTextureID()
	{
		if (mImGuiDescriptorSet == VK_NULL_HANDLE)
			return (mImGuiDescriptorSet = ImGui_ImplVulkan_AddTexture(mSampler, mpImage->imageView, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL));
		else
			return mImGuiDescriptorSet;
	}

	void VulkanTexture::GetTextureData(Ref<Buffer>& buffer)
	{
		VulkanAPI::DeviceWaitIdle();
		CommandBuffer c;
		
		VkBufferImageCopy regions{};
		regions.imageExtent.width = Width();
		regions.imageExtent.height = Height();
		regions.imageExtent.depth = 1;
		regions.bufferImageHeight = regions.bufferRowLength = 0;
		regions.bufferOffset = 0;
		regions.imageOffset = { 0, 0, 0 };
		regions.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		regions.imageSubresource.baseArrayLayer = 0;
		regions.imageSubresource.layerCount = 1;
		regions.imageSubresource.mipLevel = 0;

		mpImage->TransitionLayout(VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL);

		c.Begin();
		vkCmdCopyImageToBuffer(c, mpImage->image, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, *((VulkanBuffer*)(buffer.get())), 1, &regions);
		c.End();
		c.Submit();
		VulkanAPI::DeviceWaitIdle();
	}

	VulkanDepthTexture::VulkanDepthTexture(uint32_t width, uint32_t height)
		: Texture("Depth")
	{
		mpImage = new Image(width, height, 1, VK_FORMAT_D24_UNORM_S8_UINT, VK_SAMPLE_COUNT_1_BIT, VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, VK_IMAGE_ASPECT_DEPTH_BIT);
	}

	VulkanDepthTexture::VulkanDepthTexture(uint32_t width, uint32_t height, const TextureCreateInfo& createInfo)
		: Texture("Depth")
	{
		mpImage = new Image(width, height, 1, (VkFormat)createInfo.format, (VkSampleCountFlagBits)createInfo.sampleCount, VK_IMAGE_TILING_OPTIMAL, (VkImageUsageFlags)createInfo.usageFlags, (VkMemoryPropertyFlags)createInfo.memoryProperty, 1, 0, VK_IMAGE_VIEW_TYPE_2D, VK_IMAGE_ASPECT_DEPTH_BIT);
	}

	VulkanDepthTexture::~VulkanDepthTexture()
	{
		delete mpImage;
	}

	VulkanFramebuffer::VulkanFramebuffer(const std::vector<VkImageView>& attachments, uint32_t width, uint32_t height, VkRenderPass render_pass)
	{
		this->width = width;
		this->height = height;
		VkFramebufferCreateInfo info{};
		info.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		info.attachmentCount = (uint32_t)attachments.size();
		info.height = height;
		info.width = width;
		info.layers = 1;
		info.pAttachments = attachments.data();
		info.renderPass = render_pass;

		VkResult err = vkCreateFramebuffer(VulkanAPI::GetDevice(), &info, nullptr, &mFramebuffer);
		ET_VK_ASSERT_MSG(err, "Failed to create framebuffer!");
	}

	VulkanFramebuffer::~VulkanFramebuffer()
	{
		vkDestroyFramebuffer(VulkanAPI::GetDevice(), mFramebuffer, nullptr);
	}

	FramebufferArray::FramebufferArray(const std::vector<FramebufferCreateInfo>& createInfos)
	{
		buffers.reserve(createInfos.size());
		for (size_t i = 0; i < createInfos.size(); i++)
			buffers.push_back(CreateFramebuffer(createInfos[i]));
	}

	FramebufferArray::~FramebufferArray()
	{
		buffers.clear();
	}

	Ref<Texture> CreateTexture(const std::string& fileName, const TextureCreateInfo& createInfo)
	{
		return CreateRef<VulkanTexture>(fileName, createInfo);
	}

	Ref<Texture> CreateHDRTexture(const std::string& fileName)
	{
		return CreateRef<VulkanTexture>(fileName);
	}

	Ref<Texture> CreateTexture(const std::string& textureName,
		uint32_t width, uint32_t height,
		const TextureCreateInfo& createInfo)
	{
		return CreateRef<VulkanTexture>(textureName, width, height, createInfo);
	}

	Ref<Texture> CreateTexture(const std::string& textureName,
		uint32_t width, uint32_t height,
		const void* data,
		const TextureCreateInfo& createInfo)
	{
		return CreateRef<VulkanTexture>(textureName, width, height, data, createInfo);
	}

	Ref<Texture> CreateDepthTexture(uint32_t width, uint32_t height, const TextureCreateInfo& createInfo)
	{
		return CreateRef<VulkanDepthTexture>(width, height, createInfo);
	}

	Ref<Framebuffer> CreateFramebuffer(const FramebufferCreateInfo& createInfo)
	{
		VkRenderPass renderpass = (VkRenderPass)(*(VulkanRenderpass*)(createInfo.renderpass.get()));
		std::vector<VkImageView> attachments(createInfo.attachments.size());
		for (size_t i = 0; i < attachments.size(); i++)
		{
			VkImageView view = (VkImageView)(*(VulkanImageView*)(createInfo.attachments[i].get()));
			attachments[i] = view;
		}

		return CreateRef<VulkanFramebuffer>(attachments, createInfo.width, createInfo.height, renderpass);
	}

	Ref<FramebufferArray> CreateFramebufferArray(const std::vector<FramebufferCreateInfo>& createInfos)
	{
		return CreateRef<FramebufferArray>(createInfos);
	}
}