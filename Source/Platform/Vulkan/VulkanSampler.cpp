#include "VulkanSampler.h"

namespace Axiom {
	VulkanSampler::VulkanSampler(Vk::Device logicalDevice, const CreateInfo& createInfo) : device(logicalDevice) {
		Vk::SamplerCreateInfo samplerCreateInfo(
			{},
			axToVkFilterMode(createInfo.filterMode),
			axToVkFilterMode(createInfo.filterMode),
			axToVkMipmapMode(createInfo.mipmapFilterMode),
			axToVkAddressMode(createInfo.adressMode),
			axToVkAddressMode(createInfo.adressMode),
			axToVkAddressMode(createInfo.adressMode)
		);
		Vk::ResultValue<Vk::Sampler> samplerResult = device.createSampler(samplerCreateInfo);

		AX_CORE_ASSERT(samplerResult.result == Vk::Result::eSuccess, "Failed to create sampler!");
		sampler = samplerResult.value;
	}

	VulkanSampler::~VulkanSampler() {
		if (sampler) {
			device.destroySampler(sampler);
		}
	}
}
