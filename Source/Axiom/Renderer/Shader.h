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
		virtual ~Shader() = default;

		virtual std::vector<uint32_t> getShaderByteCode() = 0;
		virtual std::string getShaderSourceCode() = 0;

	protected:
		std::filesystem::path filePath;
	};
}

