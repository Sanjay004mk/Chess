#include "Entropy/core/application.h"
#include "Entropy/EntryPoint.h"

#include "chs_layer.h"

namespace chs
{
	class ChessApp : public et::Application
	{
	public:
		ChessApp(const et::ApplicationSpecification& specs)
			: et::Application(specs)
		{
			PushLayer(new ChessLayer());
		}
	};

}

et::Application* et::CreateApplication(et::ApplicationCommandLineArgs args)
{
	et::ApplicationSpecification specs;
	specs.ApplicationName = "Chess";
	specs.CommandLineArgs = args;

	return new chs::ChessApp(specs);
}