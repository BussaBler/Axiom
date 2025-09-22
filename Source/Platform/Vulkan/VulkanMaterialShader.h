#pragma once
#include "VulkanShader.h"
#include <vulkan/vulkan.h>

namespace Axiom {
	class VulkanMaterialShader : public VulkanShader {
	public:
		VulkanMaterialShader(VulkanDevice& vkDevice) : VulkanShader(vkDevice, "Builtin.MaterialShader") {}
		~VulkanMaterialShader() override = default;

	private:
		void createPipeline(VkRenderPass vkRenderPass) override;
	};
}

