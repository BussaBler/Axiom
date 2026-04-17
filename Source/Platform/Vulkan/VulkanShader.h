#pragma once
#include "Renderer/Shader.h"
#include "VulkanUtils.h"

namespace Axiom {
    class VulkanShader : public Shader {
      public:
        VulkanShader(Vk::Device logicalDevice, const std::string& vertexSource, const std::string& fragmentSource);
        ~VulkanShader();

        inline Vk::ShaderModule getVertexShaderModule() const { return vertexShaderModule; }
        inline Vk::ShaderModule getFragmentShaderModule() const { return fragmentShaderModule; }

      private:
        static Vk::ShaderModule createShaderModule(Vk::Device logicalDevice, const std::string& source);

      private:
        Vk::Device device;
        Vk::ShaderModule vertexShaderModule;
        Vk::ShaderModule fragmentShaderModule;
    };
} // namespace Axiom