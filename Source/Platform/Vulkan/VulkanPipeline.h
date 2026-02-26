#pragma once
#include "VulkanInclude.h"

namespace Axiom {
	class VulkanDevice;

	struct VulkanPipelineCreateInfo {
		Vk::RenderPass renderPass{};
		std::vector<Vk::VertexInputAttributeDescription> vertexAttributes;
		std::vector<Vk::DescriptorSetLayout> descriptorSetLayouts;
		std::vector<Vk::PipelineShaderStageCreateInfo> shaderStages;
		std::vector<Vk::PushConstantRange> pushConstantRanges;
		bool isWireframe = false;
	};

	class VulkanPipeline {
	public:
		VulkanPipeline(VulkanDevice& vkDevice) : device(vkDevice), pipeline(nullptr), pipelineLayout(nullptr) {}
		~VulkanPipeline();

		void init(VulkanPipelineCreateInfo& createInfo);
		void bind(Vk::CommandBuffer commandBuffer, Vk::PipelineBindPoint bindPoint) const;

		Vk::Pipeline getHandle() const { return pipeline; }
		Vk::PipelineLayout getLayout() const { return pipelineLayout; }

	private:
		VulkanDevice& device;
		Vk::Pipeline pipeline;
		Vk::PipelineLayout pipelineLayout;

	};
}

