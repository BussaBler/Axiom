#include "axpch.h"
#include "Utils/FileSystem.h"
#include "VulkanDevice.h"
#include "VulkanMaterialShader.h"
#include "Renderer/Vertex.h"

namespace Axiom {
    void VulkanMaterialShader::createPipeline(VkRenderPass vkRenderPass) {
		pipeline = std::make_unique<VulkanPipeline>(device);
		VulkanPipelineCreateInfo pipelineCreateInfo{};
		pipelineCreateInfo.renderPass = vkRenderPass;
		std::vector<VkVertexInputAttributeDescription> vertexAttributes(1);
		vertexAttributes[0].binding = 0;
		vertexAttributes[0].location = 0;
		vertexAttributes[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		vertexAttributes[0].offset = offsetof(Vertex, position);
		pipelineCreateInfo.vertexAttributes = vertexAttributes;
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts{};
		pipelineCreateInfo.descriptorSetLayouts = descriptorSetLayouts;
		for (const auto& stage : shaderStages) {
			if (stage.module != VK_NULL_HANDLE) {
				pipelineCreateInfo.shaderStages.push_back(stage.stageInfo);
			}
		}
		pipelineCreateInfo.isWireframe = false;
		pipeline->init(pipelineCreateInfo);
    }
}
