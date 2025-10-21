#include "axpch.h"
#include "Renderer/Vertex.h"
#include "Utils/FileSystem.h"
#include "VulkanBuffer.h"
#include "VulkanCommandBuffer.h"
#include "VulkanDevice.h"
#include "VulkanMaterialShader.h"
#include "VulkanTexture.h"

namespace Axiom {
	VulkanMaterialShader::~VulkanMaterialShader() {
		if (descriptorPool != VK_NULL_HANDLE) {
			vkDestroyDescriptorPool(device.getHandle(), descriptorPool, nullptr);
		}
		if (descriptorSetLayout != VK_NULL_HANDLE) {
			vkDestroyDescriptorSetLayout(device.getHandle(), descriptorSetLayout, nullptr);
		}
	}

	void VulkanMaterialShader::bindDescriptors(CommandBuffer& commandBuffer) {
		VulkanCommandBuffer& vkCommandBuffer = static_cast<VulkanCommandBuffer&>(commandBuffer);
		vkCmdBindDescriptorSets(vkCommandBuffer.getHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getLayout(), 0, 1, &descriptorSets[currentFrameIndex], 0, nullptr);
	}

	void VulkanMaterialShader::bindUniformBuffer(Resource& uniformBuffer) {
		VulkanBuffer& vkUniformBuffer = static_cast<VulkanBuffer&>(uniformBuffer);

		VkDescriptorBufferInfo globalBufferInfo{};
		globalBufferInfo.buffer = vkUniformBuffer.getHandle();
		globalBufferInfo.offset = 0;
		globalBufferInfo.range = sizeof(GlobalUbo);

		VkDescriptorBufferInfo materialBufferInfo{};
		materialBufferInfo.buffer = vkUniformBuffer.getHandle();
		materialBufferInfo.offset = sizeof(GlobalUbo);
		materialBufferInfo.range = sizeof(MaterialUbo);

		std::array<VkWriteDescriptorSet, 2> descriptorWrites{};
		descriptorWrites[0].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[0].dstSet = descriptorSets[currentFrameIndex];
		descriptorWrites[0].dstBinding = 0;
		descriptorWrites[0].dstArrayElement = 0;
		descriptorWrites[0].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[0].descriptorCount = 1;
		descriptorWrites[0].pBufferInfo = &globalBufferInfo;
		descriptorWrites[0].pImageInfo = nullptr;
		descriptorWrites[0].pTexelBufferView = nullptr;
		descriptorWrites[1].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrites[1].dstSet = descriptorSets[currentFrameIndex];
		descriptorWrites[1].dstBinding = 1;
		descriptorWrites[1].dstArrayElement = 0;
		descriptorWrites[1].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrites[1].descriptorCount = 1;
		descriptorWrites[1].pBufferInfo = &materialBufferInfo;
		descriptorWrites[1].pImageInfo = nullptr;
		descriptorWrites[1].pTexelBufferView = nullptr;
		vkUpdateDescriptorSets(device.getHandle(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	}

	void VulkanMaterialShader::bindPushConstants(CommandBuffer& commandBuffer, const void* data, uint32_t size, uint32_t offset) const {
		VulkanCommandBuffer& vkCommandBuffer = static_cast<VulkanCommandBuffer&>(commandBuffer);
		vkCmdPushConstants(vkCommandBuffer.getHandle(), pipeline->getLayout(), VK_SHADER_STAGE_VERTEX_BIT, offset, size, data);
	}

	void VulkanMaterialShader::bindTexture(Texture& texture) {
		VulkanTexture& vkTexture = static_cast<VulkanTexture&>(texture);

		VkDescriptorImageInfo imageInfo{};
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.imageView = vkTexture.getImageView();
		imageInfo.sampler = vkTexture.getHandle();

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = descriptorSets[currentFrameIndex];
		descriptorWrite.dstBinding = 2;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pImageInfo = &imageInfo;
		descriptorWrite.pBufferInfo = nullptr;
		descriptorWrite.pTexelBufferView = nullptr;
		vkUpdateDescriptorSets(device.getHandle(), 1, &descriptorWrite, 0, nullptr);
	}

	void VulkanMaterialShader::createPipeline(VkRenderPass vkRenderPass) {
		pipeline = std::make_unique<VulkanPipeline>(device);

		std::vector<VkVertexInputAttributeDescription> vertexAttributes(2);
		vertexAttributes[0].binding = 0;
		vertexAttributes[0].location = 0;
		vertexAttributes[0].format = VK_FORMAT_R32G32B32_SFLOAT;
		vertexAttributes[0].offset = offsetof(Vertex, position);
		vertexAttributes[1].binding = 0;
		vertexAttributes[1].location = 1;
		vertexAttributes[1].format = VK_FORMAT_R32G32_SFLOAT;
		vertexAttributes[1].offset = offsetof(Vertex, texCoord);
		std::vector<VkPushConstantRange> pushConstantRanges(1);
		pushConstantRanges[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pushConstantRanges[0].offset = 0;
		pushConstantRanges[0].size = sizeof(Math::Mat4);

		VulkanPipelineCreateInfo pipelineCreateInfo{};
		pipelineCreateInfo.renderPass = vkRenderPass;
		pipelineCreateInfo.vertexAttributes = vertexAttributes;
		pipelineCreateInfo.descriptorSetLayouts = std::vector<VkDescriptorSetLayout>{ descriptorSetLayout };
		pipelineCreateInfo.pushConstantRanges = pushConstantRanges;
		for (const auto& stage : shaderStages) {
			if (stage.module != VK_NULL_HANDLE) {
				pipelineCreateInfo.shaderStages.push_back(stage.stageInfo);
			}
		}
		pipelineCreateInfo.isWireframe = false;
		pipeline->init(pipelineCreateInfo);
    }

	void VulkanMaterialShader::createDescriptors() {
		VkDescriptorSetLayoutBinding globalUboLayoutBinding{};
		globalUboLayoutBinding.binding = 0;
		globalUboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		globalUboLayoutBinding.descriptorCount = 1;
		globalUboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		globalUboLayoutBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutBinding materialUboLayoutBinding{};
		materialUboLayoutBinding.binding = 1;
		materialUboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		materialUboLayoutBinding.descriptorCount = 1;
		materialUboLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		materialUboLayoutBinding.pImmutableSamplers = nullptr;

		VkDescriptorSetLayoutBinding sampler1LayoutBinding{};
		sampler1LayoutBinding.binding = 2;
		sampler1LayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		sampler1LayoutBinding.descriptorCount = 1;
		sampler1LayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		sampler1LayoutBinding.pImmutableSamplers = nullptr;

		std::array<VkDescriptorSetLayoutBinding, 3> bindings = { globalUboLayoutBinding, materialUboLayoutBinding, sampler1LayoutBinding };
		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(bindings.size());
		layoutInfo.pBindings = bindings.data();
		AX_CORE_ASSERT(vkCreateDescriptorSetLayout(device.getHandle(), &layoutInfo, nullptr, &descriptorSetLayout) == VK_SUCCESS, "Failed to create descriptor set layout!");
	
		VkDescriptorPoolSize uboPoolSize{};
		uboPoolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboPoolSize.descriptorCount = 2 * 3; // Global UBO + Material UBO * MAX_FRAMES_IN_FLIGHT
		VkDescriptorPoolSize samplerPoolSize{};
		samplerPoolSize.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		samplerPoolSize.descriptorCount = 1 * 3; // Sampler(1) * MAX_FRAMES_IN_FLIGHT
		std::array<VkDescriptorPoolSize, 2> poolSizes = { uboPoolSize, samplerPoolSize };

		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();
		poolInfo.maxSets = 3; // MAX_FRAMES_IN_FLIGHT
		AX_CORE_ASSERT(vkCreateDescriptorPool(device.getHandle(), &poolInfo, nullptr, &descriptorPool) == VK_SUCCESS, "Failed to create descriptor pool!");
	
		std::array<VkDescriptorSetLayout, 3> layouts = { descriptorSetLayout, descriptorSetLayout, descriptorSetLayout };
		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = descriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(layouts.size());
		allocInfo.pSetLayouts = layouts.data();
		descriptorSets.resize(3); // MAX_FRAMES_IN_FLIGHT
		AX_CORE_ASSERT(vkAllocateDescriptorSets(device.getHandle(), &allocInfo, descriptorSets.data()) == VK_SUCCESS, "Failed to allocate descriptor sets!");
	}
}
