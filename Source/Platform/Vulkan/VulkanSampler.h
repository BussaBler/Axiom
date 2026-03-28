#pragma once
#include "Renderer/Sampler.h"
#include "VulkanUtils.h"

namespace Axiom {
	class VulkanSampler : public Sampler {
	public:
		VulkanSampler(Vk::Device logicalDevice, const CreateInfo& createInfo);
		~VulkanSampler() override;

		inline Vk::Sampler getSampler() const { return sampler; }

	private:
		Vk::Device device = nullptr;
		Vk::Sampler sampler = nullptr;
	};
}


