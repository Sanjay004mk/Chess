#include <etpch.h>
#include <GLFW\glfw3.h>

#include "input.h"
#include "application.h"

namespace et
{
	bool Input::IsKeyDown(KeyCode key)
	{
		GLFWwindow* window = Application::Get().GetWindow().GetNativeWindow();
		int32_t state = glfwGetKey(window, key);
		return state == GLFW_PRESS;
	}

	bool Input::IsMouseButtonDown(MouseCode button)
	{
		GLFWwindow* window = Application::Get().GetWindow().GetNativeWindow();
		int32_t state = glfwGetMouseButton(window, button);
		return state == GLFW_PRESS;
	}

	glm::vec2 Input::GetMousePosition()
	{
		GLFWwindow* window = Application::Get().GetWindow().GetNativeWindow();
		double xpos, ypos;
		glfwGetCursorPos(window, &xpos, &ypos);
		return { (float)xpos, (float)ypos };

	}

	glm::vec2 Input::GetWindowPosition()
	{
		GLFWwindow* window = Application::Get().GetWindow().GetNativeWindow();
		int32_t xpos, ypos;
		glfwGetWindowPos(window, &xpos, &ypos);
		return { (float)xpos, (float)ypos };
	}

	float Input::GetMouseX()
	{
		return GetMousePosition().x;
	}

	float Input::GetMouseY()
	{
		return GetMousePosition().y;
	}

	void Input::LockMouseCursor()
	{
		GLFWwindow* window = Application::Get().GetWindow().GetNativeWindow();
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	}

	void Input::UnLockMouseCursor()
	{
		GLFWwindow* window = Application::Get().GetWindow().GetNativeWindow();
		glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	}
}