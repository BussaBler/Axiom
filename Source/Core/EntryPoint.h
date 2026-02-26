#pragma once
#include "Application.h"

extern Axiom::Application* Axiom::createApplication(const Axiom::ApplicationInfo& appInfo);

int main(int argc, char** argv) {
	Axiom::Log::init();
	auto app = Axiom::createApplication({"Axiom Application", "../../.."});
	app->run();
	delete app;
#ifdef AX_DEBUG
	std::cerr << "Press ENTER to exit..." << std::endl;
	std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
#endif // AX_DEBUG

}
