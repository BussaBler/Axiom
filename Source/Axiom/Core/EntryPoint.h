#pragma once
#include "Application.h"

extern Axiom::Application* Axiom::createApplication(const Axiom::ApplicationInfo& appInfo);

int main(int argc, char** argv) {
	Axiom::Log::init();
	auto app = Axiom::createApplication({"Axiom Application", ""});
	app->run();
	delete app;
}
