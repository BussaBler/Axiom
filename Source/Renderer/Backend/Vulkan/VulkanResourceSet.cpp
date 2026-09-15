#include "axpch.h"

#include "VulkanResourceSet.h"

#include "Renderer/ResourceLayout.h"
#include "Renderer/Texture.h"
#include "vulkan/vulkan.hpp"

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

        std::list<std::vector<Vk::DescriptorBufferInfo>> allBufferInfos;
        std::list<std::vector<Vk::DescriptorImageInfo>> allImageInfos;

        for (const auto& binding : bindings) {
            Vk::WriteDescriptorSet write{};
            write.setDstSet(descriptorSet);
            write.setDstBinding(binding.binding);
            write.setDstArrayElement(0);

            allBufferInfos.emplace_back();
            allImageInfos.emplace_back();
            auto& bufferInfos = allBufferInfos.back();
            auto& imageInfos = allImageInfos.back();

            bufferInfos.reserve(binding.buffers.size());
            imageInfos.reserve(binding.textures.size() + binding.samplers.size());

            switch (binding.type) {
            case ResourceType::UniformBuffer: {
                write.setDescriptorType(Vk::DescriptorType::eUniformBuffer);
                for (Buffer* buffer : binding.buffers) {
                    VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(buffer);
                    bufferInfos.push_back({vkBuffer->getHandle(), 0, vkBuffer->getSize()});
                }
                write.setPBufferInfo(bufferInfos.data());
                write.setDescriptorCount(static_cast<uint32_t>(bufferInfos.size()));
                break;
            }
            case ResourceType::StorageBuffer: {
                write.setDescriptorType(Vk::DescriptorType::eStorageBuffer);
                for (Buffer* buffer : binding.buffers) {
                    VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(buffer);
                    bufferInfos.push_back({vkBuffer->getHandle(), 0, vkBuffer->getSize()});
                }
                write.setPBufferInfo(bufferInfos.data());
                write.setDescriptorCount(static_cast<uint32_t>(bufferInfos.size()));
                break;
            }
            case ResourceType::Texture3D:
            case ResourceType::Texture2D: {
                write.setDescriptorType(Vk::DescriptorType::eSampledImage);
                for (Texture* texture : binding.textures) {
                    VulkanTexture* vkTexture = static_cast<VulkanTexture*>(texture);
                    imageInfos.push_back({nullptr, vkTexture->getImageView(), Vk::ImageLayout::eShaderReadOnlyOptimal});
                }
                write.setPImageInfo(imageInfos.data());
                write.setDescriptorCount(static_cast<uint32_t>(imageInfos.size()));
                break;
            }
            case ResourceType::Sampler: {
                write.setDescriptorType(Vk::DescriptorType::eSampler);
                for (Sampler* sampler : binding.samplers) {
                    VulkanSampler* vkSampler = static_cast<VulkanSampler*>(sampler);
                    imageInfos.push_back({vkSampler->getSampler(), nullptr, Vk::ImageLayout::eUndefined});
                }
                write.setPImageInfo(imageInfos.data());
                write.setDescriptorCount(static_cast<uint32_t>(imageInfos.size()));
                break;
            }
            case ResourceType::CombinedTextureSampler: {
                write.setDescriptorType(Vk::DescriptorType::eCombinedImageSampler);
                for (size_t i = 0; i < binding.textures.size() && i < binding.samplers.size(); i++) {
                    VulkanTexture* vkTexture = static_cast<VulkanTexture*>(binding.textures[i]);
                    VulkanSampler* vkSampler = static_cast<VulkanSampler*>(binding.samplers[i]);
                    imageInfos.push_back({vkSampler->getSampler(), vkTexture->getImageView(), Vk::ImageLayout::eShaderReadOnlyOptimal});
                }
                write.setPImageInfo(imageInfos.data());
                write.setDescriptorCount(static_cast<uint32_t>(imageInfos.size()));
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
