#include "axpch.h"
#include "Utils/FileSystem.h"
#include "VulkanCommandBuffer.h"
#include "VulkanDevice.h"
#include "VulkanShader.h"
#include <shaderc/shaderc.hpp>

namespace Axiom {
	std::filesystem::path VulkanShader::shadeDir = "Axiom/Assets/Shaders";

	VulkanShader::~VulkanShader() {
		for (auto& stage : shaderStages) {
			if (stage.module != VK_NULL_HANDLE) {
				vkDestroyShaderModule(device.getHandle(), stage.module, nullptr);
			}
		}
	}

	void VulkanShader::init(VkRenderPass vkRenderPass) {
		createShaderModules();
		createDescriptors();
		createPipeline(vkRenderPass);
	}

	void VulkanShader::bind(CommandBuffer& commandBuffer) const {
		pipeline->bind(static_cast<VulkanCommandBuffer&>(commandBuffer).getHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS);
	}

	void VulkanShader::createShaderModules() {
		std::array<std::string, MAX_SHADER_STAGES> shaderStageNames = { ".vert", ".frag" };
		std::array<VkShaderStageFlagBits, MAX_SHADER_STAGES> shaderStageFlags = { VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT };
		std::array<shaderc_shader_kind, MAX_SHADER_STAGES> shadercKinds = { shaderc_vertex_shader, shaderc_fragment_shader };

		for (size_t i = 0; i < MAX_SHADER_STAGES; i++) {
			std::filesystem::path fullPath = shadeDir / (shaderName + shaderStageNames[i]);
			auto src = FileSystem::readFileStr(fullPath);

			if (src.empty()) {
				continue;
			}

			shaderc::Compiler compiler{};
			shaderc::CompileOptions options{};
			shaderc_shader_kind kind = shadercKinds[i];
			auto result = compiler.CompileGlslToSpv(src, kind, fullPath.string().c_str(), "main", options);
			if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
				AX_CORE_LOG_ERROR("Failed to compile shader: {}\nError: {}", fullPath.string(), result.GetErrorMessage());
				continue;
			}

			std::vector<uint32_t> spirv(result.cbegin(), result.cend());
			shaderStages[i].createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
			shaderStages[i].createInfo.codeSize = spirv.size() * sizeof(uint32_t);
			shaderStages[i].createInfo.pCode = spirv.data();
			if (vkCreateShaderModule(device.getHandle(), &shaderStages[i].createInfo, nullptr, &shaderStages[i].module) != VK_SUCCESS) {
				AX_CORE_LOG_ERROR("Failed to create shader module for: {}", fullPath.string());
				continue;
			}
			shaderStages[i].stageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
			shaderStages[i].stageInfo.stage = shaderStageFlags[i];
			shaderStages[i].stageInfo.pName = "main";
			shaderStages[i].stageInfo.module = shaderStages[i].module;
		}
	}
}
