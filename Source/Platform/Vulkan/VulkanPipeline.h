#pragma once
#include <vulkan/vulkan.h>

namespace Axiom {
	class VulkanDevice;

	struct VulkanPipelineCreateInfo {
		VkRenderPass renderPass{};
		std::vector<VkVertexInputAttributeDescription> vertexAttributes;
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		std::vector<VkPushConstantRange> pushConstantRanges;
		bool isWireframe = false;
	};

	class VulkanPipeline {
	public:
		VulkanPipeline(VulkanDevice& vkDevice) : device(vkDevice) {}
		~VulkanPipeline();

		void init(VulkanPipelineCreateInfo& createInfo);
		void bind(VkCommandBuffer commandBuffer, VkPipelineBindPoint bindPoint) const;

		VkPipeline getHandle() const { return pipeline; }
		VkPipelineLayout getLayout() const { return pipelineLayout; }

	private:
		VulkanDevice& device;
		VkPipeline pipeline = VK_NULL_HANDLE;
		VkPipelineLayout pipelineLayout = VK_NULL_HANDLE;

	};
}

