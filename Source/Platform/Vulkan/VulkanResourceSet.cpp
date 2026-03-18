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
		std::vector<Vk::DescriptorBufferInfo> bufferInfos;
		std::vector<Vk::DescriptorImageInfo> imageInfos;

		for (const auto& binding : bindings) {
			Vk::WriteDescriptorSet write{};
			write.setDstSet(descriptorSet);
			write.setDstBinding(binding.binding);
			write.setDstArrayElement(0);
			write.setDescriptorCount(1);

			switch (binding.type) {
				case ResourceType::UniformBuffer: {
					write.setDescriptorType(Vk::DescriptorType::eUniformBuffer);
					VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(binding.buffer);
					bufferInfos.push_back({ vkBuffer->getHandle(), 0, vkBuffer->getSize() });
					write.setPBufferInfo(&bufferInfos.back());
					break;
				}
				case ResourceType::StorageBuffer: {
					write.setDescriptorType(Vk::DescriptorType::eStorageBuffer);
					VulkanBuffer* vkBuffer = static_cast<VulkanBuffer*>(binding.buffer);
					bufferInfos.push_back({ vkBuffer->getHandle(), 0, vkBuffer->getSize() });
					write.setPBufferInfo(&bufferInfos.back());
					break;
				}
				case ResourceType::TextureSampler: {
					write.setDescriptorType(Vk::DescriptorType::eCombinedImageSampler);
					VulkanTexture* vkTexture = static_cast<VulkanTexture*>(binding.texture);
					imageInfos.push_back({ vkTexture->getSampler(), vkTexture->getImageView(), Vk::ImageLayout::eShaderReadOnlyOptimal });
					write.setPImageInfo(&imageInfos.back());
					break;
				}
				default:
					break;
			}

			descriptorWrites.push_back(write);
		}

		device.updateDescriptorSets(descriptorWrites, {});
	}
}