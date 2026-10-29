#include "axpch.h"

#include "VulkanResourceSet.h"

#include "Renderer/Buffer.h"
#include "Renderer/ResourceLayout.h"
#include "Renderer/Sampler.h"
#include "Renderer/Texture.h"
#include "vulkan/vulkan.hpp"

#include <cstdint>
#include <deque>
#include <vector>

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

        std::deque<std::vector<Vk::DescriptorBufferInfo>> bufferInfosStorage;
        std::deque<std::vector<Vk::DescriptorImageInfo>> imageInfosStorage;

        for (const auto& binding : bindings) {
            Vk::WriteDescriptorSet write{};
            write.setDstSet(descriptorSet);
            write.setDstBinding(binding.binding);
            write.setDstArrayElement(binding.dataOffset);

            switch (binding.type) {
            case ResourceType::UniformBuffer: {
                write.setDescriptorType(Vk::DescriptorType::eUniformBuffer);
                const auto& buffers = std::get<std::span<Buffer*>>(binding.data);
                std::vector<Vk::DescriptorBufferInfo>& buffersInfo = bufferInfosStorage.emplace_back();
                buffersInfo.reserve(buffers.size());

                for (Buffer* buffer : buffers) {
                    VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(buffer);
                    buffersInfo.push_back({vkBuffer->getHandle(), 0, vkBuffer->getSize()});
                }
                write.setPBufferInfo(buffersInfo.data());
                write.setDescriptorCount(static_cast<uint32_t>(buffersInfo.size()));
                break;
            }
            case ResourceType::StorageBuffer: {
                write.setDescriptorType(Vk::DescriptorType::eStorageBuffer);
                const auto& buffers = std::get<std::span<Buffer*>>(binding.data);
                std::vector<Vk::DescriptorBufferInfo>& buffersInfo = bufferInfosStorage.emplace_back();
                buffersInfo.reserve(buffers.size());

                for (Buffer* buffer : buffers) {
                    VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(buffer);
                    buffersInfo.push_back({vkBuffer->getHandle(), 0, vkBuffer->getSize()});
                }
                write.setPBufferInfo(buffersInfo.data());
                write.setDescriptorCount(static_cast<uint32_t>(buffersInfo.size()));
                break;
            }
            case ResourceType::Texture3D:
            case ResourceType::Texture2D: {
                write.setDescriptorType(Vk::DescriptorType::eSampledImage);
                const auto& textures = std::get<std::span<Texture*>>(binding.data);
                std::vector<Vk::DescriptorImageInfo>& texturesInfo = imageInfosStorage.emplace_back();
                texturesInfo.reserve(textures.size());

                for (Texture* texture : textures) {
                    VulkanTexture* vkTexture = static_cast<VulkanTexture*>(texture);
                    texturesInfo.push_back({nullptr, vkTexture->getImageView(), Vk::ImageLayout::eShaderReadOnlyOptimal});
                }
                write.setPImageInfo(texturesInfo.data());
                write.setDescriptorCount(static_cast<uint32_t>(texturesInfo.size()));
                break;
            }
            case ResourceType::Sampler: {
                write.setDescriptorType(Vk::DescriptorType::eSampler);
                const auto& samplers = std::get<std::span<Sampler*>>(binding.data);
                std::vector<Vk::DescriptorImageInfo>& samplersInfo = imageInfosStorage.emplace_back();
                samplersInfo.reserve(samplers.size());

                for (Sampler* sampler : samplers) {
                    VulkanSampler* vkSampler = static_cast<VulkanSampler*>(sampler);
                    samplersInfo.push_back({vkSampler->getSampler(), nullptr, Vk::ImageLayout::eUndefined});
                }
                write.setPImageInfo(samplersInfo.data());
                write.setDescriptorCount(static_cast<uint32_t>(samplersInfo.size()));
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
