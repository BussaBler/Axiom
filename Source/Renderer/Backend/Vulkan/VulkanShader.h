#pragma once
#include "Renderer/Shader.h"
#include "VulkanUtils.h"

#include <cstdint>
#include <span>

namespace Axiom {
    class VulkanShader : public Shader {
      public:
        VulkanShader(Vk::Device logicalDevice, std::span<const uint32_t> vertexSpirv, std::span<const uint32_t> fragmentSpirv);
        ~VulkanShader();

        inline Vk::ShaderModule getVertexShaderModule() const { return vertexShaderModule; }
        inline Vk::ShaderModule getFragmentShaderModule() const { return fragmentShaderModule; }

      private:
        static Vk::ShaderModule createShaderModule(Vk::Device logicalDevice, std::span<const uint32_t> spirv);

      private:
        Vk::Device device;
        Vk::ShaderModule vertexShaderModule;
        Vk::ShaderModule fragmentShaderModule;
    };
} // namespace Axiom
