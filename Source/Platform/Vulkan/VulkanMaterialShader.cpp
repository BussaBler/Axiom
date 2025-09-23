#include "axpch.h"
#include "Renderer/Vertex.h"
#include "Utils/FileSystem.h"
#include "VulkanBuffer.h"
#include "VulkanDevice.h"
#include "VulkanMaterialShader.h"
#include "VulkanCommandBuffer.h"

namespace Axiom {
	VulkanMaterialShader::~VulkanMaterialShader() {
		if (descriptorPool != VK_NULL_HANDLE) {
			vkDestroyDescriptorPool(device.getHandle(), descriptorPool, nullptr);
		}
		if (descriptorSetLayout != VK_NULL_HANDLE) {
			vkDestroyDescriptorSetLayout(device.getHandle(), descriptorSetLayout, nullptr);
		}
	}

	void VulkanMaterialShader::bindDescriptors(CommandBuffer& commandBuffer, uint32_t offset) const {
		VulkanCommandBuffer& vkCommandBuffer = static_cast<VulkanCommandBuffer&>(commandBuffer);
		vkCmdBindDescriptorSets(vkCommandBuffer.getHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getLayout(), 0, 1, &descriptorSet, 1, &offset);
	}

	void VulkanMaterialShader::bindUniformBuffer(Resource& uniformBuffer) {
		VulkanBuffer& vkUniformBuffer = static_cast<VulkanBuffer&>(uniformBuffer);
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = vkUniformBuffer.getHandle();
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(GlobalUbo);

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = descriptorSet;
		descriptorWrite.dstBinding = 0;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfo;
		descriptorWrite.pImageInfo = nullptr;
		descriptorWrite.pTexelBufferView = nullptr;
		vkUpdateDescriptorSets(device.getHandle(), 1, &descriptorWrite, 0, nullptr);
	}

	void VulkanMaterialShader::createPipeline(VkRenderPass vkRenderPass) {
		pipeline = std::make_unique<VulkanPipeline>(device);

		std::vector<VkVertexInputAttributeDescription> vertexAttributes(1);
		vertexAttributes[0].binding = 0;
		vertexAttributes[0].location = 0;
		vertexAttributes[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		vertexAttributes[0].offset = offsetof(Vertex, position);
		std::vector<VkDescriptorSetLayout> descriptorSetLayouts(1);
		descriptorSetLayouts[0] = descriptorSetLayout;

		VulkanPipelineCreateInfo pipelineCreateInfo{};
		pipelineCreateInfo.renderPass = vkRenderPass;
		pipelineCreateInfo.vertexAttributes = vertexAttributes;
		pipelineCreateInfo.descriptorSetLayouts = descriptorSetLayouts;
		for (const auto& stage : shaderStages) {
			if (stage.module != VK_NULL_HANDLE) {
				pipelineCreateInfo.shaderStages.push_back(stage.stageInfo);
			}
		}
		pipelineCreateInfo.isWireframe = false;
		pipeline->init(pipelineCreateInfo);
    }

	void VulkanMaterialShader::createDescriptors() {
		// Global UBO
		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		uboLayoutBinding.pImmutableSamplers = nullptr;

		std::array<VkDescriptorSetLayoutBinding, 1> bindings = { uboLayoutBinding };
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();
		AX_CORE_ASSERT(vkCreateDescriptorSetLayout(device.getHandle(), &layoutInfo, nullptr, &descriptorSetLayout) == VK_SUCCESS, "Failed to create descriptor set layout!");
	
		VkDescriptorPoolSize poolSize{};
		poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
		poolSize.descriptorCount = 1;

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = 1;
		poolInfo.pPoolSizes = &poolSize;
		poolInfo.maxSets = 1;
		AX_CORE_ASSERT(vkCreateDescriptorPool(device.getHandle(), &poolInfo, nullptr, &descriptorPool) == VK_SUCCESS, "Failed to create descriptor pool!");

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = 1;
		allocInfo.pSetLayouts = &descriptorSetLayout;
		AX_CORE_ASSERT(vkAllocateDescriptorSets(device.getHandle(), &allocInfo, &descriptorSet) == VK_SUCCESS, "Failed to allocate descriptor sets!");
	}
}
