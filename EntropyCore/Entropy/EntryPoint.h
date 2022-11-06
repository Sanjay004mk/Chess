#pragma once

extern et::Application* et::CreateApplication(et::ApplicationCommandLineArgs args);

namespace et
{
	int Main(int argc, char** argv)
	{
		et::Log::Init();
		auto app = et::CreateApplication({ argc, argv });
		app->Run();
		delete app;
		return 0;
	}
}

#ifdef ET_DIST

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR lpCmdLine, INT nCmdShow)
{
	return et::Main(__argc, __argv);
}

#else

int main(int argc, char** argv)
{
	return et::Main(argc, argv);
}

#endif