#pragma once
#include "Renderer/ResourceSet.h"
#include "VulkanBuffer.h"
#include "VulkanSampler.h"
#include "VulkanTexture.h"
#include "VulkanUtils.h"

namespace Axiom {
    class VulkanResourceSet : public ResourceSet {
      public:
        VulkanResourceSet(Vk::Device logicalDevice, Vk::DescriptorPool descriptorPool, Vk::DescriptorSetLayout descriptorSetLayout);
        ~VulkanResourceSet() = default;
        void update(const std::vector<Binding>& bindings) override;

        inline Vk::DescriptorSet getHandle() const {
            return descriptorSet;
        }

      private:
        Vk::Device device = nullptr;
        Vk::DescriptorSet descriptorSet = nullptr;
    };
} // namespace Axiom
