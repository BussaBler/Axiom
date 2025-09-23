#pragma once
#include "VulkanShader.h"
#include "Renderer/GlobalUbo.h"
#include <vulkan/vulkan.h>

namespace Axiom {
	class VulkanMaterialShader : public VulkanShader {
	public:
		VulkanMaterialShader(VulkanDevice& vkDevice) : VulkanShader(vkDevice, "Builtin.MaterialShader") {}
		~VulkanMaterialShader() override;

		void bindDescriptors(CommandBuffer& commandBuffer, uint32_t offset = 0) const override;
		void bindUniformBuffer(Resource& uniformBuffer) override;

	private:
		void createPipeline(VkRenderPass vkRenderPass) override;
		void createDescriptors() override;

	private:
		VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE;
		VkDescriptorPool descriptorPool = VK_NULL_HANDLE;
		VkDescriptorSet descriptorSet = VK_NULL_HANDLE;
	};
}

