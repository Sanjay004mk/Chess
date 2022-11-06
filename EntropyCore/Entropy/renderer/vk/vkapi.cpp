#include <etpch.h>
#include <GLFW\glfw3.h>
#include <ImGui/imgui.h>
#include <ImGui/imgui_impl_vulkan.h>
#include <ImGui/imgui_impl_glfw.h>

#include "Entropy/core/application.h"
#include "Entropy/renderer/renderer.h"
#include "Entropy/scene/components.h"
#include "vkapi.h"
#include "../render_commands.h"

static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT message_severity, VkDebugUtilsMessageTypeFlagsEXT message_type, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData)
{
	if (message_severity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
		ET_LOG_ERROR("[Vulkan] DEBUG_ERROR: {0}", pCallbackData->pMessage);
	else
		ET_LOG_TRACE("[Vulkan] DEBUG_TRACE: {0}", pCallbackData->pMessage);
	return VK_FALSE;
}

namespace et
{
	VkInstance VulkanAPI::sInstance;
	VkPhysicalDevice VulkanAPI::sPhysicalDevice;
	VkDevice VulkanAPI::sDevice;
	std::optional<uint32_t> VulkanAPI::sGraphicsQueueUint;
	VkQueue VulkanAPI::sGraphicsQueue;
	SurfaceDetails VulkanAPI::sSurfaceDetails;
	VkSampleCountFlags VulkanAPI::sMaxSamples;
	uint32_t VulkanAPI::sImageIndex;
	uint32_t VulkanAPI::sCurrentFrame;

	std::vector<VkSemaphore> VulkanAPI::sImageAcquiredSempahores;
	extern VkSemaphore RenderCommand_signalSemaphore;

	void VulkanAPI::Init()
	{
		ET_LOG_TRACE("Initializing {0}...", "Vulkan");
		CreateInstance();
		SetupDevices();
		SetupDescriptorPool();
		CreateWindowSurface();
		CreateSwapchain();
		CreateRenderpass();
		InitCommandPool();
		CreateCommandBuffers();
		CreateFencesAndSemaphores();

		RenderCommand::Init();
	}

	void VulkanAPI::Shutdown()
	{
		ET_LOG_TRACE("Shutting down {0}...", "Vulkan");

		ET_LOG_TRACE("[Vulkan]: Destroying ImGui resources..");
		vkDestroyDescriptorPool(sDevice, mImGuiDescriptorPool, nullptr);
#ifdef ET_VK_ENABLE_VALIDATION
		auto vkDestroyDebugUtilsEXT = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(sInstance, "vkDestroyDebugUtilsMessengerEXT");
		vkDestroyDebugUtilsEXT(sInstance, mDebugUtilsMessenger, nullptr);
#endif
		delete mpImGuiCommandBuffers;
		delete mpImGuiCommandPools;
		mFramebuffers.clear();

		for (auto& i : mSwapchainImageViews)
			vkDestroyImageView(sDevice, i, nullptr);

		ET_LOG_TRACE("[Vulkan]: Destroying mSwapchain..");
		vkDestroySwapchainKHR(sDevice, mSwapchain, nullptr);
		vkDestroyRenderPass(sDevice, mImGuiRenderpass, nullptr);

		for (auto& s : sImageAcquiredSempahores)
			vkDestroySemaphore(sDevice, s, nullptr);
		for (auto& s : mRenderFinishedSemaphores)
			vkDestroySemaphore(sDevice, s, nullptr); 
		for (auto& f : mInFlightFences)
			vkDestroyFence(sDevice, f, nullptr);

		DestroyCommandPool();

		RenderCommand::Shutdown();

		vkDestroySurfaceKHR(sInstance, mSurface, nullptr);
		vkDestroyDevice(sDevice, nullptr);
		vkDestroyInstance(sInstance, nullptr);
	}

	void VulkanAPI::ImGuiInit()
	{
		// Setup Platform/Renderer backends
		ImGui_ImplGlfw_InitForVulkan(Application::Get().GetWindow().GetNativeWindow(), true);
		ImGui_ImplVulkan_InitInfo info = GetImGuiInitInfo();
		ImGui_ImplVulkan_Init(&info, GetImGuiRenderPass());

		UploadImGuiFonts();
	}

	void VulkanAPI::ImGuiShutdown()
	{
		ET_LOG_TRACE("Renderer Shutting down ImGui...");
		DeviceWaitIdle();

		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();
	}

	void VulkanAPI::BeginFrame()
	{
		VkResult err;

		err = vkWaitForFences(sDevice, 1, &mInFlightFences[sCurrentFrame], VK_TRUE, UINT64_MAX);
		ET_VK_ASSERT_NO_MSG(err);

		err = vkAcquireNextImageKHR(sDevice, mSwapchain, UINT64_MAX, sImageAcquiredSempahores[sCurrentFrame], VK_NULL_HANDLE, &sImageIndex);
		if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
		{
			return;
		}
		else
			ET_VK_ASSERT_MSG(err, "Failed to acquire image!");

		err = vkResetFences(sDevice, 1, &mInFlightFences[sCurrentFrame]);
		ET_VK_ASSERT_NO_MSG(err);

		RenderCommand::Begin();
	}

	void VulkanAPI::EndFrame()
	{
		VkPresentInfoKHR presentInfo{};
		presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
		presentInfo.waitSemaphoreCount = 1;
		presentInfo.pWaitSemaphores = &mRenderFinishedSemaphores[sCurrentFrame];
		presentInfo.swapchainCount = 1;
		presentInfo.pSwapchains = &mSwapchain;
		presentInfo.pImageIndices = &sImageIndex;

		VkResult err = vkQueuePresentKHR(sGraphicsQueue, &presentInfo);
		if (err == VK_ERROR_OUT_OF_DATE_KHR || err == VK_SUBOPTIMAL_KHR)
		{
			return;
		}
		else
			ET_VK_ASSERT_MSG(err, "Failed to acquire image!");


		sCurrentFrame = (sCurrentFrame + 1) % 2;
	}

	void VulkanAPI::ImGuiBegin()
	{
		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
	}

	void VulkanAPI::ImGuiEnd()
	{
		ImGuiIO& io = ImGui::GetIO();
		io.DisplaySize = ImVec2((float)Application::Get().GetWindow().GetWidth(), (float)Application::Get().GetWindow().GetHeight());

		ImGui::Render();
		ImDrawData* main_draw_data = ImGui::GetDrawData();

		VulkanAPI::ImGuiRender(main_draw_data);

		// Update and Render additional Platform Windows
		if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
		{
			ImGui::UpdatePlatformWindows();
			ImGui::RenderPlatformWindowsDefault();
		}
	}

	void VulkanAPI::CreateInstance()
	{
		ET_LOG_TRACE("[Vulkan]: Creating vulkan sInstance..");
		uint32_t extensionCount = 0;
		const char** extensions = glfwGetRequiredInstanceExtensions(&extensionCount);

		VkResult err;

		VkApplicationInfo app_info{};
		app_info.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.pApplicationName = "Entropy";
		app_info.pEngineName = "EntropyEngine";
		app_info.apiVersion = VK_API_VERSION_1_3;
		app_info.applicationVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);
		app_info.engineVersion = VK_MAKE_API_VERSION(1, 0, 0, 0);

		VkInstanceCreateInfo sInstanceCI{};
		sInstanceCI.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		sInstanceCI.enabledExtensionCount = extensionCount;
		sInstanceCI.ppEnabledExtensionNames = extensions;
		sInstanceCI.pApplicationInfo = &app_info;

#ifdef ET_VK_ENABLE_VALIDATION
		ET_LOG_TRACE("[Vulkan]: Adding debug validation layers..");
		/* add validation layers and extra extensions for debugging */
		const char* layers[] = { "VK_LAYER_KHRONOS_validation" };
		sInstanceCI.enabledLayerCount = 1;
		sInstanceCI.ppEnabledLayerNames = layers;

		const char** extensions_ext = (const char**)malloc(sizeof(const char*) * (extensionCount + 1));
		memcpy(extensions_ext, extensions, extensionCount * sizeof(const char*));
		extensions_ext[extensionCount] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;
		sInstanceCI.enabledExtensionCount = extensionCount + 1;
		sInstanceCI.ppEnabledExtensionNames = extensions_ext;

		err = vkCreateInstance(&sInstanceCI, nullptr, &sInstance);
		ET_VK_ASSERT_MSG(err, "Failed to create Vulkan sInstance!");
		free(extensions_ext);

		/* debug report callback */
		auto vkCreateDebugUtilsMessengerEXT = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(sInstance,"vkCreateDebugUtilsMessengerEXT");
		ET_ASSERT_NO_MSG(vkCreateDebugUtilsMessengerEXT != nullptr);

		VkDebugUtilsMessengerCreateInfoEXT debugCallbackCI{};
		debugCallbackCI.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		debugCallbackCI.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT;
		debugCallbackCI.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT;
		debugCallbackCI.pfnUserCallback = debug_callback;
		debugCallbackCI.pUserData = nullptr;
		
		err = vkCreateDebugUtilsMessengerEXT(sInstance, &debugCallbackCI, nullptr, &mDebugUtilsMessenger);
		ET_VK_ASSERT_MSG(err, "Failed to create Debug callback!");
#else
		err = vkCreateInstance(&sInstanceCI, nullptr, &sInstance);
		ET_VK_ASSERT_MSG(err, "Failed to create Vulkan sInstance!");
#endif
	}

	void VulkanAPI::SetupDevices()
	{
		VkResult err;
		/* physical sDevice selection */
		{
			uint32_t sDeviceCount = 0;
			err = vkEnumeratePhysicalDevices(sInstance, &sDeviceCount, nullptr);
			ET_VK_ASSERT_MSG(err, "Failed to enumerate GPUs!");
			ET_ASSERT_NO_MSG(sDeviceCount > 0);

			VkPhysicalDevice* sDevices = (VkPhysicalDevice*)malloc(sizeof(VkPhysicalDevice) * sDeviceCount);
			err = vkEnumeratePhysicalDevices(sInstance, &sDeviceCount, sDevices);
			ET_VK_ASSERT_MSG(err, "Failed to enumerate GPUs!");

			uint32_t use_gpu = 0;
			VkPhysicalDeviceProperties props{};
			for (uint32_t i = 0; i < sDeviceCount; i++)
			{
				vkGetPhysicalDeviceProperties(sDevices[i], &props);
				if (props.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
				{
					use_gpu = i;
					break;
				}
			}
			ET_LOG_INFO("[Vulkan]: Using Physical Device: {0}", props.deviceName);
			sPhysicalDevice = sDevices[use_gpu];

			// find max samples
			{
				VkPhysicalDeviceProperties physicalDeviceProperties;
				vkGetPhysicalDeviceProperties(sPhysicalDevice, &physicalDeviceProperties);

				VkSampleCountFlags counts = physicalDeviceProperties.limits.framebufferColorSampleCounts & physicalDeviceProperties.limits.framebufferDepthSampleCounts;
				if (counts & VK_SAMPLE_COUNT_64_BIT)		{ sMaxSamples = VK_SAMPLE_COUNT_64_BIT; }
				else if (counts & VK_SAMPLE_COUNT_32_BIT)	{ sMaxSamples = VK_SAMPLE_COUNT_32_BIT; }
				else if (counts & VK_SAMPLE_COUNT_16_BIT)	{ sMaxSamples = VK_SAMPLE_COUNT_16_BIT; }
				else if (counts & VK_SAMPLE_COUNT_8_BIT)	{ sMaxSamples = VK_SAMPLE_COUNT_8_BIT; }
				else if (counts & VK_SAMPLE_COUNT_4_BIT)	{ sMaxSamples = VK_SAMPLE_COUNT_4_BIT; }
				else if (counts & VK_SAMPLE_COUNT_2_BIT)	{ sMaxSamples = VK_SAMPLE_COUNT_2_BIT; }
				else sMaxSamples =  VK_SAMPLE_COUNT_1_BIT;
			}
			ET_LOG_INFO("[Vulkan]: Max supported samples: {0}", (int32_t)sMaxSamples);

			free(sDevices);
		}

		/* select graphics queue family */
		{
			uint32_t count = 0;
			vkGetPhysicalDeviceQueueFamilyProperties(sPhysicalDevice, &count, nullptr);
			VkQueueFamilyProperties* queues = (VkQueueFamilyProperties*)malloc(sizeof(VkQueueFamilyProperties) * count);
			vkGetPhysicalDeviceQueueFamilyProperties(sPhysicalDevice, &count, queues);
			for (uint32_t i = 0; i < count; i++)
			{
				if (queues[i].queueFlags & VK_QUEUE_GRAPHICS_BIT)
				{
					sGraphicsQueueUint = i;
					break;
				}
			}
			free(queues);
			ET_ASSERT_NO_MSG(sGraphicsQueueUint.has_value());
		}


		/* setup logical sDevice */
		{
			VkPhysicalDeviceFeatures features{};
			features.fillModeNonSolid = VK_TRUE;
			features.shaderSampledImageArrayDynamicIndexing = VK_TRUE;

			int32_t sDeviceExtensionCount = 1;
			const char* sDeviceExtensions[] = { "VK_KHR_swapchain" };
			float queuePriority[] = { 1.0f };

			VkDeviceQueueCreateInfo queueCI{};
			queueCI.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCI.queueFamilyIndex = sGraphicsQueueUint.value();
			queueCI.queueCount = 1;
			queueCI.pQueuePriorities = queuePriority;
			
			VkDeviceCreateInfo sDeviceCI{};
			sDeviceCI.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
			sDeviceCI.queueCreateInfoCount = 1;
			sDeviceCI.pQueueCreateInfos = &queueCI;
			sDeviceCI.enabledExtensionCount = sDeviceExtensionCount;
			sDeviceCI.ppEnabledExtensionNames = sDeviceExtensions;
			sDeviceCI.pEnabledFeatures = &features;

			err = vkCreateDevice(sPhysicalDevice, &sDeviceCI, nullptr, &sDevice);
			ET_VK_ASSERT_MSG(err, "Failed to create logical sDevice!");
			vkGetDeviceQueue(sDevice, sGraphicsQueueUint.value(), 0, &sGraphicsQueue);
		}
	}

	void VulkanAPI::SetupDescriptorPool()
	{
		ET_LOG_TRACE("[Vulkan]: Setting up ImGui Descriptor pool..");
		VkResult err;
		VkDescriptorPoolSize poolSizes[] =
		{
			{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
			{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
			{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
			{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
		};

		VkDescriptorPoolCreateInfo poolCI{};
		poolCI.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolCI.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		poolCI.maxSets = 1000 * (uint32_t)(sizeof(poolSizes) / sizeof(VkDescriptorPoolSize));
		poolCI.poolSizeCount = (uint32_t)(sizeof(poolSizes) / sizeof(VkDescriptorPoolSize));
		poolCI.pPoolSizes = poolSizes;
		
		err = vkCreateDescriptorPool(sDevice, &poolCI, nullptr, &mImGuiDescriptorPool);
		ET_VK_ASSERT_MSG(err, "Failed to create descriptor pool!");
	}

	void VulkanAPI::CreateWindowSurface()
	{
		VkResult err;
		{
			err = glfwCreateWindowSurface(sInstance, Application::Get().GetWindow().GetNativeWindow(), nullptr, &mSurface);
			ET_VK_ASSERT_MSG(err, "Failed to create window mSurface!");

			VkBool32 res;
			vkGetPhysicalDeviceSurfaceSupportKHR(sPhysicalDevice, sGraphicsQueueUint.value(), mSurface, &res);
			ET_ASSERT_MSG_BREAK(res == VK_TRUE, "[Vulkan] error: GPU doesn't support WSI!");			
		}

	}

	void VulkanAPI::CreateSwapchain()
	{
		ET_LOG_TRACE("[Vulkan]: Creating mSwapchain..");
		VkResult err;

		VkSurfaceFormatKHR useFormat;
		/* select mSurface format */
		{
			VkFormat formats[] = { VK_FORMAT_B8G8R8A8_UNORM, VK_FORMAT_R8G8B8A8_SNORM, VK_FORMAT_B8G8R8_UNORM, VK_FORMAT_R8G8B8_SNORM };
			VkColorSpaceKHR colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;

			uint32_t count = 0;
			vkGetPhysicalDeviceSurfaceFormatsKHR(sPhysicalDevice, mSurface, &count, nullptr);
			VkSurfaceFormatKHR* mSurfaceFormat = (VkSurfaceFormatKHR*)malloc(sizeof(VkSurfaceFormatKHR) * count);
			vkGetPhysicalDeviceSurfaceFormatsKHR(sPhysicalDevice, mSurface, &count, mSurfaceFormat);

			if (count == 1)
			{
				if (mSurfaceFormat[0].format == VK_FORMAT_UNDEFINED)
				{
					useFormat.format = formats[0];
					useFormat.colorSpace = colorSpace;
				}
				else
				{
					useFormat = mSurfaceFormat[0];
				}

			}
			else
			{
				bool set = false;
				for (size_t i = 0; i < 4; i++)
				{
					for (uint32_t j = 0; j < count; j++)
						if (mSurfaceFormat[j].format == formats[i] && mSurfaceFormat[j].colorSpace == colorSpace)
						{
							useFormat = mSurfaceFormat[j];
							set = true;
							break;
						}
					if (set)
						break;

				}
				if (!set)
					useFormat = mSurfaceFormat[0];
			}

			free(mSurfaceFormat);
		}

		VkPresentModeKHR presentMode;
		/* select present mode */
		{
			if (Application::Get().GetWindow().IsVSync())
			{
				presentMode = VK_PRESENT_MODE_FIFO_KHR;
				ET_LOG_TRACE("[Vulkan]: Present mode '{0}'", "FIFO");
			}
			else
			{
				VkPresentModeKHR modes[] = { VK_PRESENT_MODE_MAILBOX_KHR, VK_PRESENT_MODE_FIFO_KHR, VK_PRESENT_MODE_IMMEDIATE_KHR };
				uint32_t count = 0;
				vkGetPhysicalDeviceSurfacePresentModesKHR(sPhysicalDevice, mSurface, &count, nullptr);
				VkPresentModeKHR* avail = (VkPresentModeKHR*)malloc(sizeof(VkPresentModeKHR) * count);
				vkGetPhysicalDeviceSurfacePresentModesKHR(sPhysicalDevice, mSurface, &count, avail);

				bool set = false;
				for (size_t i = 0; i < 3; i++)
				{
					for (uint32_t j = 0; j < count; j++)
						if (avail[j] == modes[i])
						{
							presentMode = modes[i];
							set = true;
							switch (i)
							{
							case 0:
								ET_LOG_TRACE("[Vulkan]: Present mode '{0}'", "MAILBOX");
								break;
							case 1:
								ET_LOG_TRACE("[Vulkan]: Present mode '{0}'", "IMMEDIATE");
								break;
							case 2:
								ET_LOG_TRACE("[Vulkan]: Present mode '{0}'", "FIFO");
								break;

							}
							break;
						}
					if (set)
						break;
				}
				if (!set)
				{
					presentMode = VK_PRESENT_MODE_FIFO_KHR;
					ET_LOG_TRACE("[Vulkan]: Present mode '{0}'", "FIFO");
				}

				free(avail);
			}
		}
		VkSurfaceCapabilitiesKHR capabilites{};
		err = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(sPhysicalDevice, mSurface, &capabilites);
		ET_VK_ASSERT_MSG(err, "Failed to fetch mSurface capablities!");
		sSurfaceDetails.imageCount = capabilites.minImageCount;

		/* select extent */
		VkExtent2D extent{};
		if (capabilites.currentExtent.width != UINT32_MAX)
			extent = capabilites.currentExtent;
		else
		{
			int32_t width, height;
			glfwGetFramebufferSize(Application::Get().GetWindow().GetNativeWindow(), &width, &height);
			extent = { (uint32_t)width, (uint32_t)height };
			extent.width = std::clamp(extent.width, capabilites.minImageExtent.width, capabilites.maxImageExtent.width);
			extent.height = std::clamp(extent.height, capabilites.minImageExtent.height, capabilites.maxImageExtent.height);
		}

		/* create mSwapchain */
		VkSwapchainCreateInfoKHR mSwapchainCI{};
		mSwapchainCI.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
		mSwapchainCI.clipped = VK_TRUE;
		mSwapchainCI.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		mSwapchainCI.imageArrayLayers = 1;
		mSwapchainCI.imageColorSpace = useFormat.colorSpace;
		mSwapchainCI.imageExtent = extent;
		mSwapchainCI.imageFormat = useFormat.format;
		mSwapchainCI.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
		mSwapchainCI.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		mSwapchainCI.presentMode = presentMode;
		mSwapchainCI.minImageCount = sSurfaceDetails.imageCount;
		mSwapchainCI.queueFamilyIndexCount = 0;
		mSwapchainCI.pQueueFamilyIndices = nullptr;
		mSwapchainCI.oldSwapchain = VK_NULL_HANDLE;
		mSwapchainCI.preTransform = capabilites.currentTransform;
		mSwapchainCI.surface = mSurface;

		err = vkCreateSwapchainKHR(sDevice, &mSwapchainCI, nullptr, &mSwapchain);
		ET_VK_ASSERT_MSG(err, "Failed to create mSwapchain!");

		uint32_t imageCount = 0;
		err = vkGetSwapchainImagesKHR(sDevice, mSwapchain, &imageCount, nullptr);
		ET_VK_ASSERT_MSG(err, "Failed to fetch mSwapchain images!");
		mSwapchainImages.resize(imageCount);
		mSwapchainImageViews.resize(imageCount);
		err = vkGetSwapchainImagesKHR(sDevice, mSwapchain, &imageCount, mSwapchainImages.data());
		ET_VK_ASSERT_MSG(err, "Failed to fetch mSwapchain images!");

		VkImageViewCreateInfo ivci{};
		ivci.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		ivci.format = useFormat.format;
		ivci.viewType = VK_IMAGE_VIEW_TYPE_2D;
		ivci.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		ivci.subresourceRange.baseArrayLayer = 0;
		ivci.subresourceRange.baseMipLevel = 0;
		ivci.subresourceRange.layerCount = 1;
		ivci.subresourceRange.levelCount = 1;

		for (size_t i = 0; i < mSwapchainImages.size(); i++)
		{
			ivci.image = mSwapchainImages[i];

			err = vkCreateImageView(sDevice, &ivci, nullptr, &mSwapchainImageViews[i]);
			ET_VK_ASSERT_MSG(err, "Failed to create image view!");
		}

		sSurfaceDetails.colorSpace = useFormat.colorSpace;
		sSurfaceDetails.format = useFormat.format;
		sSurfaceDetails.extent = extent;
	}

	void VulkanAPI::CreateRenderpass()
	{
		ET_LOG_TRACE("[Vulkan]: Creating ImGui Renderpass..");
		VkResult err;

		/* create renderpass */
		VkAttachmentDescription attachment = {};
		attachment.format = sSurfaceDetails.format;
		attachment.samples = VK_SAMPLE_COUNT_1_BIT;
		attachment.loadOp = VK_ATTACHMENT_LOAD_OP_LOAD;
		attachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
		attachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
		attachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
		attachment.initialLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
		attachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		VkAttachmentReference color_attachment = {};
		color_attachment.attachment = 0;
		color_attachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

		VkSubpassDescription subpass = {};
		subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1;
		subpass.pColorAttachments = &color_attachment;

		VkSubpassDependency dependency = {};
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0;
		dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0;
		dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

		VkRenderPassCreateInfo info = {};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
		info.attachmentCount = 1;
		info.pAttachments = &attachment;
		info.subpassCount = 1;
		info.pSubpasses = &subpass;
		info.dependencyCount = 1;
		info.pDependencies = &dependency;
		err = vkCreateRenderPass(sDevice, &info, nullptr, &mImGuiRenderpass);
		ET_VK_ASSERT_MSG(err, "Failed to create render pass!");

		/* create framebuffers */
		uint32_t width = sSurfaceDetails.extent.width;
		uint32_t height = sSurfaceDetails.extent.height;

		std::vector<std::vector<VkImageView>> attachments;
		
		for (uint32_t i = 0; i < sSurfaceDetails.imageCount; i++)
			attachments.push_back({ mSwapchainImageViews[i] });

		mFramebuffers.resize(sSurfaceDetails.imageCount);
		for (size_t i = 0; i < mFramebuffers.size(); i++)
		{
			mFramebuffers[i] = CreateRef<VulkanFramebuffer>(attachments[i], width, height, mImGuiRenderpass);
		}
	}

	void VulkanAPI::Resize()
	{
		ET_LOG_TRACE("[Vulkan]: Destroying mSwapchain..");
		DeviceWaitIdle();

		mFramebuffers.clear();

		for (auto& i : mSwapchainImageViews)
			vkDestroyImageView(sDevice, i, nullptr);

		vkDestroySwapchainKHR(sDevice, mSwapchain, nullptr);
		vkDestroyRenderPass(sDevice, mImGuiRenderpass, nullptr);

		CreateSwapchain();
		CreateRenderpass();

		sCurrentFrame = 0;
	}

	void VulkanAPI::CreateCommandBuffers()
	{
		mpImGuiCommandPools = new CommandPoolArray(sSurfaceDetails.imageCount);
		mpImGuiCommandBuffers = new CommandBufferArray(*mpImGuiCommandPools);
	}

	void VulkanAPI::CreateFencesAndSemaphores()
	{
		sImageAcquiredSempahores.resize(sSurfaceDetails.imageCount);
		mRenderFinishedSemaphores.resize(sSurfaceDetails.imageCount);
		mInFlightFences.resize(2);

		VkSemaphoreCreateInfo sci{};
		sci.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

		VkFenceCreateInfo fci{};
		fci.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fci.flags = VK_FENCE_CREATE_SIGNALED_BIT;

		VkResult err;
		for (size_t i = 0; i < 2; i++)
		{
			err = vkCreateSemaphore(sDevice, &sci, nullptr, &sImageAcquiredSempahores[i]);
			ET_VK_ASSERT_MSG(err, "Failed to create semaphores!");

			err = vkCreateSemaphore(sDevice, &sci, nullptr, &mRenderFinishedSemaphores[i]);
			ET_VK_ASSERT_MSG(err, "Failed to create semaphores!");

			err = vkCreateFence(sDevice, &fci, nullptr, &mInFlightFences[i]);
			ET_VK_ASSERT_MSG(err, "Failed to create fences!");
		}
	}

	void VulkanAPI::DeviceWaitIdle()
	{
		VkResult err = vkDeviceWaitIdle(sDevice);
		ET_VK_ASSERT_NO_MSG(err);
	}

	ImGui_ImplVulkan_InitInfo VulkanAPI::GetImGuiInitInfo()
	{
		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = sInstance;
		init_info.PhysicalDevice = sPhysicalDevice;
		init_info.Device = sDevice;
		init_info.QueueFamily = sGraphicsQueueUint.value();
		init_info.Queue = sGraphicsQueue;
		init_info.PipelineCache = VK_NULL_HANDLE;
		init_info.DescriptorPool = mImGuiDescriptorPool;
		init_info.Subpass = 0;
		init_info.MinImageCount = sSurfaceDetails.imageCount;
		init_info.ImageCount = sSurfaceDetails.imageCount;
		init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;
		init_info.Allocator = nullptr;
		init_info.CheckVkResultFn = [](VkResult err)
		{
			ET_VK_ASSERT_NO_MSG(err);
		};

		return init_info;
	}

	void VulkanAPI::UploadImGuiFonts()
	{
		CommandBuffer cb;
		cb.Begin();

		ImGui_ImplVulkan_CreateFontsTexture(cb);

		cb.End();
		cb.Submit();
		DeviceWaitIdle();

		ImGui_ImplVulkan_DestroyFontUploadObjects();
	}

	void VulkanAPI::Present(Ref<Texture> image)
	{
		presentImage = image;
	}

	void VulkanAPI::ImGuiRender(ImDrawData* data)
	{
		RenderCommand::End();

		mpImGuiCommandPools->Reset(sImageIndex);
		mpImGuiCommandBuffers->Begin(sImageIndex);
		auto& cb = (*mpImGuiCommandBuffers)[sImageIndex];

		// clear image
		VkClearColorValue clear_color = { 0.1f, 0.1f, 0.1f, 1.0f };

		VkImageSubresourceRange image_subresource_range = {
			VK_IMAGE_ASPECT_COLOR_BIT,                    // aspectMask
			0,                                            // baseMipLevel
			1,                                            // levelCount
			0,                                            // baseArrayLayer
			1                                             // layerCount
		};

		VkImageMemoryBarrier barrier_from_present_to_clear = {
			VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,     // sType
			nullptr,                                    // pNext
			VK_ACCESS_MEMORY_READ_BIT,                  // srcAccessMask
			VK_ACCESS_TRANSFER_WRITE_BIT,               // dstAccessMask
			VK_IMAGE_LAYOUT_UNDEFINED,                  // oldLayout
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,       // newLayout
			VK_QUEUE_FAMILY_IGNORED,				    // srcQueueFamilyIndex
			VK_QUEUE_FAMILY_IGNORED,			        // dstQueueFamilyIndex
			mSwapchainImages[sImageIndex],              // image
			image_subresource_range                     // subresourceRange
		};

		VkImageMemoryBarrier barrier_from_clear_to_present = {
				VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,     // sType
				nullptr,                                    // pNext
				VK_ACCESS_TRANSFER_WRITE_BIT,               // srcAccessMask
				VK_ACCESS_MEMORY_READ_BIT,                  // dstAccessMask
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,       // oldLayout
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,   // newLayout
				VK_QUEUE_FAMILY_IGNORED,                    // srcQueueFamilyIndex
				VK_QUEUE_FAMILY_IGNORED,                    // dstQueueFamilyIndex
				mSwapchainImages[sImageIndex],              // image
				image_subresource_range                     // subresourceRange
		};

		vkCmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier_from_present_to_clear);

		vkCmdClearColorImage(cb, mSwapchainImages[sImageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, &clear_color, 1, &image_subresource_range);

		// present image
		if (presentImage)
		{
			if (presentImage->Width() == sSurfaceDetails.extent.width &&
				presentImage->Height() == sSurfaceDetails.extent.height &&
				(VkFormat)presentImage->Format() == sSurfaceDetails.format)
			{
				VkImageCopy copyRegion{};
				copyRegion.dstOffset = copyRegion.srcOffset = { 0 };
				copyRegion.extent = { sSurfaceDetails.extent.width, sSurfaceDetails.extent.height, 1 };
				copyRegion.dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				copyRegion.dstSubresource.baseArrayLayer = 0;
				copyRegion.dstSubresource.layerCount = 1;
				copyRegion.dstSubresource.mipLevel = 0;
				copyRegion.srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
				copyRegion.srcSubresource.baseArrayLayer = 0;
				copyRegion.srcSubresource.layerCount = 1;
				copyRegion.srcSubresource.mipLevel = 0;

				VkImage p = *(VulkanTexture*)presentImage.get();

				vkCmdCopyImage(cb, p, VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL, mSwapchainImages[sImageIndex], VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &copyRegion);
			}
		}
		vkCmdPipelineBarrier(cb, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT, 0, 0, nullptr, 0, nullptr, 1, &barrier_from_clear_to_present);

		VkRenderPassBeginInfo info{};
		info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
		info.framebuffer = *(mFramebuffers[sCurrentFrame]);
		info.renderPass = mImGuiRenderpass;
		info.renderArea.extent = sSurfaceDetails.extent;
		info.clearValueCount = 0;
		info.pClearValues = nullptr;

		vkCmdBeginRenderPass((*mpImGuiCommandBuffers)[sImageIndex], &info, VK_SUBPASS_CONTENTS_INLINE);

		ImGui_ImplVulkan_RenderDrawData(data, (*mpImGuiCommandBuffers)[sImageIndex]);

		vkCmdEndRenderPass((*mpImGuiCommandBuffers)[sImageIndex]);

		VkPipelineStageFlags waitStage = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;

		mpImGuiCommandBuffers->End(sImageIndex);

		(*mpImGuiCommandBuffers)[sImageIndex].Submit(&RenderCommand_signalSemaphore, 1, &waitStage, &mRenderFinishedSemaphores[sCurrentFrame], 1, mInFlightFences[sCurrentFrame]);

		presentImage.reset();
	}

	uint32_t VulkanAPI::GetMemoryTypeIndex(uint32_t type_filter, VkMemoryPropertyFlags property_flags)
	{
		VkPhysicalDeviceMemoryProperties memProperties;
		vkGetPhysicalDeviceMemoryProperties(sPhysicalDevice, &memProperties);

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
			if ((type_filter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & property_flags) == property_flags) {
				return i;
			}
		}

		ET_ASSERT_MSG_BREAK(false, "failed to find suitable memory type!");
		return 0;
	}

	std::string VkResultToString(VkResult result)
	{
		switch (result)
		{
		case VK_SUCCESS:
			return "VK_SUCCESS";
			break;
		case VK_NOT_READY:
			return "VK_NOT_READY";
			break;
		case VK_TIMEOUT:
			return "VK_TIMEOUT";
			break;
		case VK_EVENT_SET:
			return "VK_EVENT_SET";
			break;
		case VK_EVENT_RESET:
			return "VK_EVENT_RESET";
			break;
		case VK_INCOMPLETE:
			return "VK_IMCOMPLETE";
			break;
		case VK_ERROR_OUT_OF_HOST_MEMORY:
			return "VK_ERROR_OUT_OF_HOST_MEMORY";
			break;
		case VK_ERROR_OUT_OF_DEVICE_MEMORY:
			return "VK_ERROR_OUT_OF_DEVICE_MEMORY";
			break;
		case VK_ERROR_INITIALIZATION_FAILED:
			return "VK_ERROR_INITIALIZATION_FAILED";
			break;
		case VK_ERROR_DEVICE_LOST:
			return "VK_ERROR_DEVICE_LOST";
			break;
		case VK_ERROR_MEMORY_MAP_FAILED:
			return "VK_ERROR_MEMORY_MAP_FAILED";
			break;
		case VK_ERROR_LAYER_NOT_PRESENT:
			return "VK_ERROR_LAYER_NOT_PRESENT";
			break;
		case VK_ERROR_EXTENSION_NOT_PRESENT:
			return "VK_ERROR_EXTENSION_NOT_PRESENT";
			break;
		case VK_ERROR_FEATURE_NOT_PRESENT:
			return "VK_ERROR_FEATURE_NOT_PRESENT";
			break;
		case VK_ERROR_INCOMPATIBLE_DRIVER:
			return "VK_ERROR_INCOMPATIBLE_DRIVER";
			break;
		case VK_ERROR_TOO_MANY_OBJECTS:
			return "VK_ERROR_TOO_MANY_OBJECTS";
			break;
		case VK_ERROR_FORMAT_NOT_SUPPORTED:
			return "VK_ERROR_FORMAT_NOT_SUPPORTED";
			break;
		case VK_ERROR_SURFACE_LOST_KHR:
			return "VK_ERROR_SURFACE_LOST_KHR";
			break;
		case VK_SUBOPTIMAL_KHR:
			return "VK_SUBOPTIMAL_KHR";
			break;
		case VK_ERROR_OUT_OF_DATE_KHR:
			return "VK_ERROR_OUT_OF_DATE_KHR";
			break;
		case VK_ERROR_INCOMPATIBLE_DISPLAY_KHR:
			return "VK_ERROR_INCOMPATIBLE_DISPLAY_KHR";
			break;
		case VK_ERROR_NATIVE_WINDOW_IN_USE_KHR:
			return "VK_ERROR_NATIVE_WINDOW_IN_USE_KHR";
			break;
		case VK_RESULT_MAX_ENUM:
			return "VK_RESULT_MAX_ENUM";
			break;
		default:
			return "VK_RESULT_UNKNOWN";
			break;
		}
	}

	std::string VkDebugReportObjectTypeEXTToString(VkDebugReportObjectTypeEXT o)
	{
		switch (o)
		{
		case VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT: 
			return "VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN_EXT";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT:      
			return "VK_DEBUG_REPORT_OBJECT_TYPE_INSTANCE_EXT";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT:                          
			return "VK_DEBUG_REPORT_OBJECT_TYPE_PHYSICAL_DEVICE_EXT";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT:                                   
			return "VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_EXT";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT:                                    
			return "VK_DEBUG_REPORT_OBJECT_TYPE_QUEUE_EXT";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT:                                
			return "VK_DEBUG_REPORT_OBJECT_TYPE_SEMAPHORE_EXT";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT:                           
			return "VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_BUFFER_EXT";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT:                                    
			return "VK_DEBUG_REPORT_OBJECT_TYPE_FENCE_EXT";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT:                            
			return "VK_DEBUG_REPORT_OBJECT_TYPE_DEVICE_MEMORY_EXT";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT:                                   
			return "VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_EXT";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT:                                    
			return "VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_EXT";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT:                                    
			return "VK_DEBUG_REPORT_OBJECT_TYPE_EVENT_EXT";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT:                               
			return "VK_DEBUG_REPORT_OBJECT_TYPE_QUERY_POOL_EXT";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT:                              
			return "VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_VIEW_EXT";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT:                               
			return "VK_DEBUG_REPORT_OBJECT_TYPE_IMAGE_VIEW_EXT";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT:                            
			return "VK_DEBUG_REPORT_OBJECT_TYPE_SHADER_MODULE_EXT";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_CACHE_EXT:                           
			return "VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_CACHE_EXT";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT:                          
			return "VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_LAYOUT_EXT";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT:                              
			return "VK_DEBUG_REPORT_OBJECT_TYPE_RENDER_PASS_EXT";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT:                                 
			return "VK_DEBUG_REPORT_OBJECT_TYPE_PIPELINE_EXT";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT:                    
			return "VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_LAYOUT_EXT";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT:                                  
			return "VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_EXT";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT:                          
			return "VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_POOL_EXT";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT:                           
			return "VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_SET_EXT";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT:                              
			return "VK_DEBUG_REPORT_OBJECT_TYPE_FRAMEBUFFER_EXT";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT:                             
			return "VK_DEBUG_REPORT_OBJECT_TYPE_COMMAND_POOL_EXT";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_SURFACE_KHR_EXT:                              
			return "VK_DEBUG_REPORT_OBJECT_TYPE_SURFACE_KHR_EXT";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT:                            
			return "VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_DISPLAY_KHR_EXT:                              
			return "VK_DEBUG_REPORT_OBJECT_TYPE_DISPLAY_KHR_EXT";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_DISPLAY_MODE_KHR_EXT:                         
			return "VK_DEBUG_REPORT_OBJECT_TYPE_DISPLAY_MODE_KHR_EXT";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_VALIDATION_CACHE_EXT:                     
			return "VK_DEBUG_REPORT_OBJECT_TYPE_VALIDATION_CACHE_EXT";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION_EXT:                 
			return "VK_DEBUG_REPORT_OBJECT_TYPE_SAMPLER_YCBCR_CONVERSION_EXT";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_EXT:               
			return "VK_DEBUG_REPORT_OBJECT_TYPE_DESCRIPTOR_UPDATE_TEMPLATE_EXT";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_CU_MODULE_NVX_EXT:                            
			return "VK_DEBUG_REPORT_OBJECT_TYPE_CU_MODULE_NVX_EXT";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_CU_FUNCTION_NVX_EXT:                          
			return "VK_DEBUG_REPORT_OBJECT_TYPE_CU_FUNCTION_NVX_EXT";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR_EXT:               
			return "VK_DEBUG_REPORT_OBJECT_TYPE_ACCELERATION_STRUCTURE_KHR_EXT";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV_EXT:                
			return "VK_DEBUG_REPORT_OBJECT_TYPE_ACCELERATION_STRUCTURE_NV_EXT";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_COLLECTION_FUCHSIA_EXT:                
			return "VK_DEBUG_REPORT_OBJECT_TYPE_BUFFER_COLLECTION_FUCHSIA_EXT";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_DEBUG_REPORT_EXT:                            
			return "VK_DEBUG_REPORT_OBJECT_TYPE_DEBUG_REPORT_EXT";
			break;
		case VK_DEBUG_REPORT_OBJECT_TYPE_MAX_ENUM_EXT:                                 
			return "VK_DEBUG_REPORT_OBJECT_TYPE_MAX_ENUM_EXT";
			break;
		default:
			return "VK_DEBUG_REPORT_OBJECT_TYPE_UNKNOWN";
			break;
		}
	}
}
