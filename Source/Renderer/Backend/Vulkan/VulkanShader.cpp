#include "axpch.h"

#include "VulkanShader.h"

#include <cstdint>
#include <span>

namespace Axiom {
    VulkanShader::VulkanShader(Vk::Device logicalDevice, std::span<const uint32_t> vertexSpirv, std::span<const uint32_t> fragmentSpirv)
        : device(logicalDevice) {
        vertexShaderModule = createShaderModule(logicalDevice, vertexSpirv);
        fragmentShaderModule = createShaderModule(logicalDevice, fragmentSpirv);
        m_isCompiled = true;
    }

    VulkanShader::~VulkanShader() {
        if (vertexShaderModule) {
            device.destroyShaderModule(vertexShaderModule);
        }
        if (fragmentShaderModule) {
            device.destroyShaderModule(fragmentShaderModule);
        }
    }

    Vk::ShaderModule VulkanShader::createShaderModule(Vk::Device logicalDevice, std::span<const uint32_t> spirv) {
        Vk::ShaderModuleCreateInfo shaderModuleCreateInfo({}, spirv.size() * sizeof(uint32_t), spirv.data());
        Vk::ResultValue<Vk::ShaderModule> shaderModuleResult = logicalDevice.createShaderModule(shaderModuleCreateInfo);

        AX_CORE_ASSERT(shaderModuleResult.result == Vk::Result::eSuccess, "Failed to create shader module!");
        return shaderModuleResult.value;
    }
} // namespace Axiom
