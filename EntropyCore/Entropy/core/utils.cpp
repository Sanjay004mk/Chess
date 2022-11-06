#include <etpch.h>
#include <commdlg.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "utils.h"
#include "application.h"

namespace et
{
	std::string FileDialogs::OpenFile(std::string_view filter)
	{
		CHAR szFile[260] = {0};
		CHAR currentDir[256] = {0};
		OPENFILENAMEA ofn{};
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.Flags = OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST;
		ofn.hwndOwner = glfwGetWin32Window(Application::Get().GetWindow().GetNativeWindow());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		if (GetCurrentDirectoryA(256, currentDir))
			ofn.lpstrInitialDir = currentDir;
		ofn.lpstrFilter = filter.data();
		ofn.nFilterIndex = 1;

		if (GetOpenFileNameA(&ofn) == TRUE)
			return ofn.lpstrFile;

		return {};

	}

	std::string FileDialogs::SaveFile(std::string_view filter)
	{
		CHAR szFile[260] = { 0 };
		CHAR currentDir[256] = { 0 };
		OPENFILENAMEA ofn{};
		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.Flags = OFN_NOCHANGEDIR | OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
		ofn.hwndOwner = glfwGetWin32Window(Application::Get().GetWindow().GetNativeWindow());
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		if (GetCurrentDirectoryA(256, currentDir))
			ofn.lpstrInitialDir = currentDir;
		ofn.lpstrFilter = filter.data();
		ofn.nFilterIndex = 1;
		ofn.lpstrDefExt = strchr(filter.data(), 0) + 1;

		if (GetSaveFileNameA(&ofn) == TRUE)
			return ofn.lpstrFile;

		return {};
	}
}