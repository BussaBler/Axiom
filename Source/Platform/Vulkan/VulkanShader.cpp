#include "VulkanShader.h"
#include <shaderc/shaderc.hpp>

namespace Axiom {
    VulkanShader::VulkanShader(Vk::Device logicalDevice, const std::string& vertexSource, const std::string& fragmentSource) : device(logicalDevice) {
        vertexShaderModule = createShaderModule(logicalDevice, vertexSource);
        fragmentShaderModule = createShaderModule(logicalDevice, fragmentSource);
    }

    VulkanShader::~VulkanShader() {
        if (vertexShaderModule) {
            device.destroyShaderModule(vertexShaderModule);
        }
        if (fragmentShaderModule) {
            device.destroyShaderModule(fragmentShaderModule);
        }
    }

    Vk::ShaderModule VulkanShader::createShaderModule(Vk::Device logicalDevice, const std::string& source) {
        shaderc::Compiler compiler = {};
        shaderc::CompileOptions options = {};
        options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_4);
        shaderc::CompilationResult compResult =
            compiler.CompileGlslToSpv(source.c_str(), source.size(), shaderc_shader_kind::shaderc_glsl_infer_from_source, "shader", "main", options);
        AX_CORE_ASSERT(compResult.GetCompilationStatus() == shaderc_compilation_status_success, "Failed to compile shader: {0}", compResult.GetErrorMessage());

        size_t codeSize = std::distance(compResult.cbegin(), compResult.cend()) * sizeof(uint32_t);
        const uint32_t* shaderCode = compResult.cbegin();

        Vk::ShaderModuleCreateInfo shaderModuleCreateInfo({}, codeSize, shaderCode);
        Vk::ResultValue<Vk::ShaderModule> shaderModuleResult = logicalDevice.createShaderModule(shaderModuleCreateInfo);

        AX_CORE_ASSERT(shaderModuleResult.result == Vk::Result::eSuccess, "Failed to create shader module!");
        return shaderModuleResult.value;
    }
} // namespace Axiom