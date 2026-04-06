#include "VulkanResourceLayout.h"

namespace Axiom {
    VulkanResourceLayout::VulkanResourceLayout(Vk::Device logicalDevice, const std::vector<BindingCreateInfo> &bindings) : device(logicalDevice) {
        std::vector<Vk::DescriptorSetLayoutBinding> layoutBindings;

        for (const auto &bindingCreateInfo : bindings) {
            Vk::DescriptorSetLayoutBinding layoutBinding{};
            layoutBinding.setBinding(bindingCreateInfo.binding);
            layoutBinding.setDescriptorCount(bindingCreateInfo.count);
            layoutBinding.setDescriptorType(axToVkDescriptorType(bindingCreateInfo.type));
            layoutBinding.setStageFlags(axToVkShaderStageFlags(bindingCreateInfo.stages));

            layoutBindings.push_back(layoutBinding);
        }

        Vk::DescriptorSetLayoutCreateInfo layoutCreateInfo({}, layoutBindings);
        Vk::ResultValue<Vk::DescriptorSetLayout> layoutResult = device.createDescriptorSetLayout(layoutCreateInfo);

        AX_CORE_ASSERT(layoutResult.result == Vk::Result::eSuccess, "Failed to create Vulkan descriptor set layout!");
        descriptorSetLayout = layoutResult.value;
    }

    VulkanResourceLayout::~VulkanResourceLayout() {
        if (descriptorSetLayout) {
            device.destroyDescriptorSetLayout(descriptorSetLayout);
        }
    }
} // namespace Axiom
