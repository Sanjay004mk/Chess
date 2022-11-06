#include <etpch.h>

#include <GLFW\glfw3.h>
#ifdef ET_PLATFORM_WINDOWS
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

#include "Entropy/events/application_events.h"
#include "Entropy/events/key_events.h"
#include "Entropy/events/mouse_events.h"
#include "window.h"
#include "time_utils.h"

namespace et
{
	static uint8_t sWindowCount = 0;

	static void GlfwErrorCallback(int err, const char* description)
	{
		ET_LOG_ERROR("[GLFW]: error {0}: {1}", err, description);
	}

	Window::Window(const WindowProperties& properties)
	{
		Init(properties);
	}

	Window::~Window()
	{
		Shutdown();
	}

	void Window::Init(const WindowProperties& properties)
	{
		mData.Title = properties.Title;
		mData.Width = properties.Width;
		mData.Height = properties.Height;

		if (sWindowCount == 0)
		{
			ET_LOG_TRACE("Initializing {}...", "GLFW");
			int32_t result = glfwInit();
			ET_ASSERT_NO_MSG(result == GLFW_TRUE);
			glfwSetErrorCallback(GlfwErrorCallback);
			result = glfwVulkanSupported();
			ET_ASSERT_NO_MSG_BREAK(result == GLFW_TRUE);
		}

		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		mpWindow = glfwCreateWindow(mData.Width, mData.Height, mData.Title.c_str(), nullptr, nullptr);
		sWindowCount++;
		glfwSetWindowSizeLimits(mpWindow, 600, 600, GLFW_DONT_CARE, GLFW_DONT_CARE);
		glfwSetWindowUserPointer(mpWindow, &mData);
		SetVSync(false);
		ET_LOG_TRACE("Creating Window '{0}' [{1}, {2}]", mData.Title, mData.Width, mData.Height);

		glfwSetFramebufferSizeCallback(mpWindow, [](GLFWwindow* window, int32_t width, int32_t height)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				data.Width = (uint32_t)width;
				data.Height = (uint32_t)height;

				WindowResizeEvent event(data.Width, data.Height);
				data.callback(event);
			});

		glfwSetWindowCloseCallback(mpWindow, [](GLFWwindow* window)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				WindowCloseEvent event;
				data.callback(event);

			});

		glfwSetKeyCallback(mpWindow, [](GLFWwindow* window, int32_t key, int32_t scancode, int32_t action, int32_t mods)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				switch (action)
				{
				case GLFW_PRESS:
				{
					KeyPressedEvent event(key);
					data.callback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					KeyReleasedEvent event(key);
					data.callback(event);
					break;
				}
				case GLFW_REPEAT:
				{
					KeyPressedEvent event(key, true);
					data.callback(event);
					break;
				}
				}

			});

		glfwSetCharCallback(mpWindow, [](GLFWwindow* window, uint32_t keycode)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				KeyTypedEvent event(keycode);
				data.callback(event);
			});

		glfwSetMouseButtonCallback(mpWindow, [](GLFWwindow* window, int32_t button, int32_t action, int32_t mods)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

				switch (action)
				{
				case GLFW_PRESS:
				{
					MouseButtonPressedEvent event(button);
					data.callback(event);
					break;
				}
				case GLFW_RELEASE:
				{
					MouseButtonReleasedEvent event(button);
					data.callback(event);
					break;
				}
				}
			});

		glfwSetScrollCallback(mpWindow, [](GLFWwindow* window, double xOffs, double yOffs)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				MouseScrolledEvent event((float)xOffs, (float)yOffs);
				data.callback(event);
			});

		glfwSetCursorPosCallback(mpWindow, [](GLFWwindow* window, double xPos, double yPos)
			{
				WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
				MouseMovedEvent event((float)xPos, (float)yPos);
				data.callback(event);
			});
	}

	void Window::Shutdown()
	{
		ET_LOG_TRACE("Destroying window '{0}'", mData.Title);
		glfwDestroyWindow(mpWindow);
		--sWindowCount;
		if (sWindowCount == 0)
		{
			ET_LOG_TRACE("Shutting down GLFW...");
			glfwTerminate();
		}
	}

	void Window::OnUpdate()
	{
		glfwPollEvents();
	}
}