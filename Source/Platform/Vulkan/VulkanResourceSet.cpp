#include "VulkanResourceSet.h"

namespace Axiom {
    VulkanResourceSet::VulkanResourceSet(Vk::Device logicalDevice, Vk::DescriptorPool descriptorPool, Vk::DescriptorSetLayout descriptorSetLayout)
        : device(logicalDevice) {
        Vk::DescriptorSetAllocateInfo allocInfo(descriptorPool, descriptorSetLayout);
        Vk::ResultValue<std::vector<Vk::DescriptorSet>> descriptorSetResult = device.allocateDescriptorSets(allocInfo);

        AX_CORE_ASSERT(descriptorSetResult.result == Vk::Result::eSuccess, "Failed to allocate descriptor set!");
        descriptorSet = descriptorSetResult.value[0];
    }

    void VulkanResourceSet::update(const std::vector<Binding>& bindings) {
        std::vector<Vk::WriteDescriptorSet> descriptorWrites;
        descriptorWrites.reserve(bindings.size());

        for (const auto& binding : bindings) {
            Vk::WriteDescriptorSet write{};
            write.setDstSet(descriptorSet);
            write.setDstBinding(binding.binding);
            write.setDstArrayElement(0);
            write.setDescriptorCount(1);

            std::vector<Vk::DescriptorBufferInfo> bufferInfos;
            std::vector<Vk::DescriptorImageInfo> imageInfos;
            bufferInfos.reserve(binding.buffers.size());
            imageInfos.reserve(binding.textures.size() + binding.samplers.size());

            switch (binding.type) {
            case ResourceType::UniformBuffer: {
                for (Buffer* buffer : binding.buffers) {
                    write.setDescriptorType(Vk::DescriptorType::eUniformBuffer);
                    VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(buffer);
                    bufferInfos.push_back({vkBuffer->getHandle(), 0, vkBuffer->getSize()});
                    write.setPBufferInfo(&bufferInfos.back());
                }
                break;
            }
            case ResourceType::StorageBuffer: {
                for (Buffer* buffer : binding.buffers) {
                    write.setDescriptorType(Vk::DescriptorType::eStorageBuffer);
                    VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(buffer);
                    bufferInfos.push_back({vkBuffer->getHandle(), 0, vkBuffer->getSize()});
                    write.setPBufferInfo(&bufferInfos.back());
                }
                break;
            }
            case ResourceType::Texture: {
                for (Texture* texture : binding.textures) {
                    write.setDescriptorType(Vk::DescriptorType::eSampledImage);
                    VulkanTexture* vkTexture = static_cast<VulkanTexture*>(texture);
                    imageInfos.push_back({nullptr, vkTexture->getImageView(), Vk::ImageLayout::eShaderReadOnlyOptimal});
                    write.setPImageInfo(&imageInfos.back());
                }
                break;
            }
            case ResourceType::Sampler: {
                for (Sampler* sampler : binding.samplers) {
                    write.setDescriptorType(Vk::DescriptorType::eSampler);
                    VulkanSampler* vkSampler = static_cast<VulkanSampler*>(sampler);
                    imageInfos.push_back({vkSampler->getSampler(), nullptr, Vk::ImageLayout::eUndefined});
                    write.setPImageInfo(&imageInfos.back());
                }
                break;
            }
            case ResourceType::CombinedTextureSampler: {
                for (size_t i = 0; i < binding.textures.size() && i < binding.samplers.size(); i++) {
                    write.setDescriptorType(Vk::DescriptorType::eCombinedImageSampler);
                    VulkanTexture* vkTexture = static_cast<VulkanTexture*>(binding.textures[i]);
                    VulkanSampler* vkSampler = static_cast<VulkanSampler*>(binding.samplers[i]);
                    imageInfos.push_back({vkSampler->getSampler(), vkTexture->getImageView(), Vk::ImageLayout::eShaderReadOnlyOptimal});
                    write.setPImageInfo(&imageInfos.back());
                }
                break;
            }
            default:
                break;
            }

            descriptorWrites.push_back(write);
        }

        device.updateDescriptorSets(descriptorWrites, {});
    }
} // namespace Axiom