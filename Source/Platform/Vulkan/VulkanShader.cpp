#include "axpch.h"
#include "Utils/FileSystem.h"
#include "VulkanCommandBuffer.h"
#include "VulkanDevice.h"
#include "VulkanShader.h"
#include <shaderc/shaderc.hpp>

namespace Axiom {
	std::filesystem::path VulkanShader::shadeDir = "Assets/Shaders";

	VulkanShader::~VulkanShader() {
		for (auto& stage : shaderStages) {
			if (stage.module) {
				device.getHandle().destroyShaderModule(stage.module);
			}
		}
	}

	void VulkanShader::init(Vk::RenderPass vkRenderPass) {
		createShaderModules();
		createDescriptors();
		createPipeline(vkRenderPass);
	}

	void VulkanShader::bind(CommandBuffer& commandBuffer) const {
		pipeline->bind(static_cast<VulkanCommandBuffer&>(commandBuffer).getHandle(), Vk::PipelineBindPoint::eGraphics);
	}

	void VulkanShader::createShaderModules() {
		std::array<std::string, MAX_SHADER_STAGES> shaderStageNames = { ".vert", ".frag" };
		std::array<Vk::ShaderStageFlagBits, MAX_SHADER_STAGES> shaderStageFlags = { Vk::ShaderStageFlagBits::eVertex, Vk::ShaderStageFlagBits::eFragment };
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
			shaderStages[i].createInfo.setCodeSize(spirv.size() * sizeof(uint32_t));
			shaderStages[i].createInfo.setCode(spirv);
			Vk::ResultValue<Vk::ShaderModule> shaderModuleResult = device.getHandle().createShaderModule(shaderStages[i].createInfo);

			if (shaderModuleResult.result != Vk::Result::eSuccess) {
				AX_CORE_LOG_ERROR("Failed to create shader module for: {}", fullPath.string());
				continue;
			}
			shaderStages[i].module = shaderModuleResult.value;

			shaderStages[i].stageInfo.setStage(shaderStageFlags[i]);
			shaderStages[i].stageInfo.setPName("main");
			shaderStages[i].stageInfo.setModule(shaderStages[i].module);
		}
	}
}
