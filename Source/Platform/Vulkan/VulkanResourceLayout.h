#pragma once
#include "Renderer/ResourceLayout.h"
#include "VulkanUtils.h"

namespace Axiom {
	class VulkanResourceLayout : public ResourceLayout {
	public:
		VulkanResourceLayout(Vk::Device logicalDevice, const std::vector<BindingCreateInfo>& bindings);
		~VulkanResourceLayout() override;

		inline Vk::DescriptorSetLayout getHandle() const { return descriptorSetLayout; }

	private:
		Vk::Device device = nullptr;
		Vk::DescriptorSetLayout descriptorSetLayout = nullptr;
	};
}

