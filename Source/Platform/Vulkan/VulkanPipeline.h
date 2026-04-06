#pragma once
#include "Core/Assert.h"
#include "Renderer/Pipeline.h"
#include "VulkanResourceLayout.h"
#include "VulkanUtils.h"

namespace Axiom {
    class VulkanPipeline : public Pipeline {
      public:
        VulkanPipeline(const CreateInfo &createInfo, Vk::Device logicDevice, Vk::DescriptorPool descriptorPool);
        ~VulkanPipeline() override;

        std::unique_ptr<ResourceSet> createResourceSet(ResourceLayout *resourceLayout) override;

        inline Vk::Pipeline getHandle() const {
            return pipeline;
        }
        inline Vk::PipelineLayout getPipelineLayout() const {
            return pipelineLayout;
        }

      private:
        Vk::ShaderModule createShaderModule(std::filesystem::path shaderPath);
        Vk::PolygonMode axPolygonToVkPolygon(PolygonMode mode);
        Vk::CullModeFlags axCullModeToVkCullMode(CullMode mode);

      private:
        Vk::Device device = nullptr;
        Vk::DescriptorPool descriptorPool = nullptr;
        Vk::Pipeline pipeline = nullptr;
        Vk::PipelineLayout pipelineLayout = nullptr;
    };
} // namespace Axiom
