#pragma once
#include "VulkanDevice.h"
#include "VulkanRenderPass.h"
#include "Renderer/Data/Geometry/Vertex.h"
#include "Renderer/Core/Pipeline.h"

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

	class VulkanPipeline : public Pipeline {
	public:
		VulkanPipeline(VulkanDevice& device, RenderPass& renderPass, const PipelineConfigInfo& configInfo);
		~VulkanPipeline();
		void bind(CommandBuffer& commandBuffer, PipelineBindPoint bindPoint) override;

		VkPipelineLayout getLayout() const { return layout; }
	private:
		VkPipelineLayout layout;
		VulkanDevice& device;
	};
}

