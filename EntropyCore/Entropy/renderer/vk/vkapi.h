#pragma once
#include <optional>
#include <vulkan\vulkan.h>
#include <vector>

#include "Entropy/renderer/renderer_api.h"
#include "Entropy/core/log.h"
#include "vk_buffer.h"
#include "vk_render_pass.h"
#include "vk_pipeline.h"
#include "vk_shader.h"
#include "vk_texture.h"

namespace et
{

	std::string VkResultToString(VkResult result);
	std::string VkDebugReportObjectTypeEXTToString(VkDebugReportObjectTypeEXT o);

}
#define ET_VK_ASSERT_MSG(code, msg) ET_ASSERT_MSG(code == VK_SUCCESS, "[Vulkan] error {1}: {0}", msg, ::et::VkResultToString(code))
#define ET_VK_ASSERT_NO_MSG(code) ET_ASSERT_NO_MSG(code == VK_SUCCESS)

#if defined(ET_DEBUG)// || defined(ET_RELEASE)
#define ET_VK_ENABLE_VALIDATION
#endif

struct ImGui_ImplVulkan_InitInfo;
struct ImDrawData;

namespace et
{
	struct SurfaceDetails
	{
		VkFormat format;
		VkColorSpaceKHR colorSpace;
		VkExtent2D extent;
		uint32_t imageCount;
	};

	class VulkanAPI : public RenderAPI
	{
	public:
		virtual API GetAPI() const override { return API::Vulkan; }

		virtual void Init() override;
		virtual void Shutdown() override;

		virtual void ImGuiInit() override;
		virtual void ImGuiShutdown() override;

		virtual void BeginFrame() override;
		virtual void ImGuiBegin() override;
		virtual void EndFrame() override;
		virtual void ImGuiEnd() override;

		virtual void Resize() override;

		virtual TextureFormat GetSurfaceFormat() const override { return (TextureFormat)sSurfaceDetails.format; }

		virtual void Present(Ref<Texture> image) override;

		ImGui_ImplVulkan_InitInfo GetImGuiInitInfo();
		void UploadImGuiFonts();
		void ImGuiRender(ImDrawData* data);

		VkRenderPass GetImGuiRenderPass() { return mImGuiRenderpass; }
		static void DeviceWaitIdle();

		static VkDevice GetDevice() { return sDevice; }
		static VkPhysicalDevice GetPhysicalDevice() { return sPhysicalDevice; }
		static VkQueue GetGraphicsQueue() { return sGraphicsQueue; }
		static uint32_t GetQueueIndex() { return sGraphicsQueueUint.value(); }
		static uint32_t GetCurrentFrame() { return sCurrentFrame; }
		static SurfaceDetails& GetSurfaceDetails() { return sSurfaceDetails; }
		static SampleCount GetMaxSampleCount() { return (SampleCount)sMaxSamples; }

		static uint32_t GetMemoryTypeIndex(uint32_t type_filter, VkMemoryPropertyFlags property_flags);

	private:
		void CreateInstance();
		void SetupDevices();
		void SetupDescriptorPool();
		void CreateWindowSurface();
		void CreateSwapchain();
		void CreateRenderpass();
		void CreateCommandBuffers();
		void CreateFencesAndSemaphores();

		static SurfaceDetails sSurfaceDetails;
		static VkSampleCountFlags sMaxSamples;

		static uint32_t sImageIndex;
		static uint32_t sCurrentFrame;

		static VkInstance sInstance;
		static VkPhysicalDevice sPhysicalDevice;
		static VkDevice sDevice;
		static std::optional<uint32_t> sGraphicsQueueUint;
		static VkQueue sGraphicsQueue;
		static std::vector<VkSemaphore> sImageAcquiredSempahores;

		VkSurfaceKHR mSurface;
		VkDescriptorPool mImGuiDescriptorPool;
		CommandPoolArray* mpImGuiCommandPools;
		CommandBufferArray* mpImGuiCommandBuffers;

		VkSwapchainKHR mSwapchain;
		std::vector<VkImage> mSwapchainImages;
		std::vector<VkImageView> mSwapchainImageViews;
		std::vector<Ref<VulkanFramebuffer>> mFramebuffers; 
		VkRenderPass mImGuiRenderpass;

		static Ref<Texture> presentImage;

		std::vector<VkSemaphore> mRenderFinishedSemaphores;
		std::vector<VkFence> mInFlightFences;
#ifdef ET_VK_ENABLE_VALIDATION
		VkDebugUtilsMessengerEXT mDebugUtilsMessenger;
#endif
		friend class RenderCommand;
		friend class ImGuiLayer;
	};
}