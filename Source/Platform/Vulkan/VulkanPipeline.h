#pragma once
#include "VulkanDevice.h"
#include "VulkanRenderPass.h"
#include "Renderer/Vertex.h"

namespace Axiom {
	struct PipelineConfigInfo {
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions;
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts;
		std::vector<VkPipelineShaderStageCreateInfo> shaderStages;
		std::vector<VkPushConstantRange> pushConstantsRange;
		VkViewport viewport;
		VkRect2D scissor;
		bool isWireframe = false;
	};

	class VulkanPipeline {
	public:
		VulkanPipeline(VulkanDevice& device, VulkanRenderPass& renderPass, const PipelineConfigInfo& configInfo);
		virtual ~VulkanPipeline();
		virtual void bind(VulkanCommandBuffer& commandBuffer, VkPipelineBindPoint bindPoint);

		VkPipeline getHandle() const { return handle; }
		VkPipelineLayout getLayout() const { return layout; }
	private:
		VkPipeline handle;
		VkPipelineLayout layout;
		VulkanDevice& device;
	};
}

