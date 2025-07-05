#pragma once
#include "Application.h"

extern Axiom::Application* Axiom::createApplication();

int main(int argc, char** argv) {
	Axiom::Log::init();
	auto app = Axiom::createApplication();
	app->run();
	delete app;
}
