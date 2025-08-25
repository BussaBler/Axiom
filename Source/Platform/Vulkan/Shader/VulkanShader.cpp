#include "axpch.h"
#include "VulkanShader.h"

namespace Axiom {
	VulkanShader::VulkanShader(VulkanDevice& vkDevice) : device(vkDevice) {
		shaderDir = std::filesystem::current_path() / "Axiom" / "Assets" / "Shaders";
	}

	VulkanShader::~VulkanShader() {
		for (auto& stage : shaderStages) {
			if (stage.handle != VK_NULL_HANDLE) {
				vkDestroyShaderModule(device.getHandle(), stage.handle, nullptr);
			}
		}
	}

	void VulkanShader::createShaderModule(const std::string& shaderName, const std::string& shaderStageStr, VkShaderStageFlagBits shaderStageFlag, uint32_t index) {
		std::string shaderFileName = shaderName + "." + shaderStageStr;

		auto shaderCode = getShaderByteCode(shaderDir / shaderFileName, shaderStageFlag);

		shaderStages[index].createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderStages[index].createInfo.codeSize = shaderCode.size() * sizeof(uint32_t);
		shaderStages[index].createInfo.pCode = shaderCode.data();

		AX_CORE_ASSERT(vkCreateShaderModule(device.getHandle(), &shaderStages[index].createInfo, nullptr, &shaderStages[index].handle) == VK_SUCCESS,
			"Failed to create shader module for shader: {0}", shaderFileName);

		shaderStages[index].stageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[index].stageCreateInfo.stage = shaderStageFlag;
		shaderStages[index].stageCreateInfo.module = shaderStages[index].handle;
		shaderStages[index].stageCreateInfo.pName = "main"; // Entry point name
	}

	std::vector<uint32_t> VulkanShader::getShaderByteCode(std::filesystem::path filePath, VkShaderStageFlagBits shaderStageFlag) {
		std::string sourceCode = FileSystem::readFileStr(filePath);
		shaderc::Compiler compiler;

		shaderc_shader_kind kind;
		switch (shaderStageFlag) {
			case VK_SHADER_STAGE_VERTEX_BIT:
				kind = shaderc_glsl_default_vertex_shader;
				break;
			case VK_SHADER_STAGE_FRAGMENT_BIT:
				kind = shaderc_glsl_default_fragment_shader;
				break;
			default:
				kind = shaderc_glsl_default_vertex_shader;
		}
		auto result = compiler.CompileGlslToSpv(sourceCode, kind, filePath.string().c_str());

		if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
			AX_CORE_LOG_ERROR("Shader compilation failed: {0}", result.GetErrorMessage());
			return {};
		}

		return { result.begin(), result.end() };
	}
}