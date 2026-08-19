#pragma once
#include "Application.h"

extern Axiom::Application* Axiom::createApplication(int argc, char** argv, const std::filesystem::path& axiomRootDir);

int main(int argc, char** argv) {
    auto app = Axiom::createApplication(argc, argv, std::filesystem::path(AX_ROOT_DIR));
    app->run();
    delete app;
#ifdef AX_DEBUG
    std::cin.get();
#endif
}
