#include "axpch.h"
#include "VulkanShader.h"

namespace Axiom {
	VulkanShader::VulkanShader(std::filesystem::path& filePath) : Shader(filePath) {}

	std::vector<uint32_t> VulkanShader::getShaderByteCode() {
		std::string sourceCode = getShaderSourceCode();
		shaderc::Compiler compiler;

		auto result = compiler.CompileGlslToSpv(sourceCode, shaderc_glsl_default_vertex_shader, filePath.string().c_str());

		if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
			AX_CORE_LOG_ERROR("Shader compilation failed: {0}", result.GetErrorMessage());
			return {};
		}

		return { result.begin(), result.end() };
	}

	std::string VulkanShader::getShaderSourceCode() {
		std::ifstream file(filePath, std::ios::ate);
		if (!file.is_open()) {
			AX_CORE_LOG_ERROR("Failed to open shader file: {0}", filePath.string());
			return {};
		}

		size_t fileSize = static_cast<size_t>(file.tellg());
		std::string buffer(fileSize, '\0');
		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();

		return buffer;
	}
}