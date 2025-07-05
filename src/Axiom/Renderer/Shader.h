#pragma once
#include "Core/Log.h"
#include <filesystem>
#include <fstream>
#include <shaderc/shaderc.hpp>
#include <string>
#include <vector>

namespace Axiom {
	class Shader {
	public:
		Shader(std::filesystem::path filePath);
		~Shader() = default;

		std::vector<uint32_t> getShaderByteCode();
		std::string getShaderSourceCode();

	private:
		std::filesystem::path filePath;
	};
}

