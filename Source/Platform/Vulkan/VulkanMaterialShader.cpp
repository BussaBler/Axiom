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
		if (descriptorPool) {
			device.getHandle().destroyDescriptorPool(descriptorPool);
		}
		if (descriptorSetLayout) {
			device.getHandle().destroyDescriptorSetLayout(descriptorSetLayout);
		}
	}

	void VulkanMaterialShader::bindDescriptors(CommandBuffer& commandBuffer) {
		VulkanCommandBuffer& vkCommandBuffer = static_cast<VulkanCommandBuffer&>(commandBuffer);
		vkCommandBuffer.getHandle().bindDescriptorSets(Vk::PipelineBindPoint::eGraphics, pipeline->getLayout(), 0, descriptorSets[currentFrameIndex], {});
	}

	void VulkanMaterialShader::bindUniformBuffer(Resource& uniformBuffer) {
		VulkanBuffer& vkUniformBuffer = static_cast<VulkanBuffer&>(uniformBuffer);

		Vk::DescriptorBufferInfo globalBufferInfo(vkUniformBuffer.getHandle(), 0, sizeof(GlobalUbo));

		Vk::DescriptorBufferInfo materialBufferInfo(vkUniformBuffer.getHandle(), sizeof(GlobalUbo), sizeof(MaterialUbo));

		std::array<Vk::WriteDescriptorSet, 2> descriptorWrites{};
		descriptorWrites[0].setDstSet(descriptorSets[currentFrameIndex]);
		descriptorWrites[0].setDstBinding(0);
		descriptorWrites[0].setDstArrayElement(0);
		descriptorWrites[0].setDescriptorType(Vk::DescriptorType::eUniformBuffer);
		descriptorWrites[0].setDescriptorCount(1);
		descriptorWrites[0].setBufferInfo(globalBufferInfo);
		descriptorWrites[1].setDstSet(descriptorSets[currentFrameIndex]);
		descriptorWrites[1].setDstBinding(1);
		descriptorWrites[1].setDstArrayElement(0);
		descriptorWrites[1].setDescriptorType(Vk::DescriptorType::eUniformBuffer);
		descriptorWrites[1].setDescriptorCount(1);
		descriptorWrites[1].setBufferInfo(materialBufferInfo);
		std::array<Vk::CopyDescriptorSet, 0> descriptorCopies{};
		
		device.getHandle().updateDescriptorSets(descriptorWrites, descriptorCopies);
	}

	void VulkanMaterialShader::bindPushConstants(CommandBuffer& commandBuffer, const void* data, uint32_t size, uint32_t offset) const {
		VulkanCommandBuffer& vkCommandBuffer = static_cast<VulkanCommandBuffer&>(commandBuffer);
		vkCommandBuffer.getHandle().pushConstants(pipeline->getLayout(), Vk::ShaderStageFlagBits::eVertex, offset, size, data);
	}

	void VulkanMaterialShader::bindTexture(Texture& texture) {
		VulkanTexture& vkTexture = static_cast<VulkanTexture&>(texture);

		Vk::DescriptorImageInfo imageInfo(vkTexture.getHandle(), vkTexture.getImageView(), Vk::ImageLayout::eShaderReadOnlyOptimal);

		Vk::WriteDescriptorSet descriptorWrite{};
		descriptorWrite.setDstSet(descriptorSets[currentFrameIndex]);
		descriptorWrite.setDstBinding(2);
		descriptorWrite.setDstArrayElement(0);
		descriptorWrite.setDescriptorType(Vk::DescriptorType::eCombinedImageSampler);
		descriptorWrite.setDescriptorCount(1);
		descriptorWrite.setImageInfo(imageInfo);
		device.getHandle().updateDescriptorSets(descriptorWrite, {});
	}

	void VulkanMaterialShader::createPipeline(Vk::RenderPass vkRenderPass) {
		pipeline = std::make_unique<VulkanPipeline>(device);

		std::vector<Vk::VertexInputAttributeDescription> vertexAttributes(2);
		vertexAttributes[0].setBinding(0);
		vertexAttributes[0].setLocation(0);
		vertexAttributes[0].setFormat(Vk::Format::eR32G32B32Sfloat);
		vertexAttributes[0].setOffset(offsetof(Vertex, position));
		vertexAttributes[1].setBinding(0);
		vertexAttributes[1].setLocation(1);
		vertexAttributes[1].setFormat(Vk::Format::eR32G32Sfloat);
		vertexAttributes[1].setOffset(offsetof(Vertex, texCoord));
		std::vector<Vk::PushConstantRange> pushConstantRanges(1);
		pushConstantRanges[0].setStageFlags(Vk::ShaderStageFlagBits::eVertex);
		pushConstantRanges[0].setOffset(0);
		pushConstantRanges[0].setSize(sizeof(Math::Mat4));

		VulkanPipelineCreateInfo pipelineCreateInfo{};
		pipelineCreateInfo.renderPass = vkRenderPass;
		pipelineCreateInfo.vertexAttributes = vertexAttributes;
		pipelineCreateInfo.descriptorSetLayouts = std::vector<Vk::DescriptorSetLayout>{ descriptorSetLayout };
		pipelineCreateInfo.pushConstantRanges = pushConstantRanges;
		for (const auto& stage : shaderStages) {
			if (stage.module) {
				pipelineCreateInfo.shaderStages.push_back(stage.stageInfo);
			}
		}
		pipelineCreateInfo.isWireframe = false;
		pipeline->init(pipelineCreateInfo);
    }

	void VulkanMaterialShader::createDescriptors() {
		Vk::DescriptorSetLayoutBinding globalUboLayoutBinding{};
		globalUboLayoutBinding.setBinding(0);
		globalUboLayoutBinding.setDescriptorType(Vk::DescriptorType::eUniformBuffer);
		globalUboLayoutBinding.setDescriptorCount(1);
		globalUboLayoutBinding.setStageFlags(Vk::ShaderStageFlagBits::eVertex);

		Vk::DescriptorSetLayoutBinding materialUboLayoutBinding{};
		materialUboLayoutBinding.setBinding(1);
		materialUboLayoutBinding.setDescriptorType(Vk::DescriptorType::eUniformBuffer);
		materialUboLayoutBinding.setDescriptorCount(1);
		materialUboLayoutBinding.setStageFlags(Vk::ShaderStageFlagBits::eFragment);

		Vk::DescriptorSetLayoutBinding sampler1LayoutBinding{};
		sampler1LayoutBinding.setBinding(2);
		sampler1LayoutBinding.setDescriptorType(Vk::DescriptorType::eCombinedImageSampler);
		sampler1LayoutBinding.setDescriptorCount(1);
		sampler1LayoutBinding.setStageFlags(Vk::ShaderStageFlagBits::eFragment);

		std::array<Vk::DescriptorSetLayoutBinding, 3> bindings = { globalUboLayoutBinding, materialUboLayoutBinding, sampler1LayoutBinding };
		Vk::DescriptorSetLayoutCreateInfo layoutInfo({}, bindings);
		Vk::ResultValue<Vk::DescriptorSetLayout> descriptorSetLayoutResult = device.getHandle().createDescriptorSetLayout(layoutInfo);

		AX_CORE_ASSERT(descriptorSetLayoutResult.result == Vk::Result::eSuccess, "Failed to create descriptor set layout!");
		descriptorSetLayout = descriptorSetLayoutResult.value;
	
		Vk::DescriptorPoolSize uboPoolSize(Vk::DescriptorType::eUniformBuffer, 2 * 3); // Global UBO + Material UBO * MAX_FRAMES_IN_FLIGHT
		Vk::DescriptorPoolSize samplerPoolSize(Vk::DescriptorType::eCombinedImageSampler, 1 * 3); // Sampler(1) * MAX_FRAMES_IN_FLIGHT
		std::array<Vk::DescriptorPoolSize, 2> poolSizes = { uboPoolSize, samplerPoolSize };

		Vk::DescriptorPoolCreateInfo poolInfo({}, 3, poolSizes); // MAX_FRAMES_IN_FLIGHT
		Vk::ResultValue<Vk::DescriptorPool> descriptorPoolResult = device.getHandle().createDescriptorPool(poolInfo);

		AX_CORE_ASSERT(descriptorPoolResult.result == Vk::Result::eSuccess, "Failed to create descriptor pool!");
		descriptorPool = descriptorPoolResult.value;
	
		std::array<Vk::DescriptorSetLayout, 3> layouts = { descriptorSetLayout, descriptorSetLayout, descriptorSetLayout };
		Vk::DescriptorSetAllocateInfo allocInfo(descriptorPool, layouts);
		Vk::ResultValue<std::vector<Vk::DescriptorSet>> descriptorSetsResult = device.getHandle().allocateDescriptorSets(allocInfo);

		AX_CORE_ASSERT(descriptorSetsResult.result == Vk::Result::eSuccess, "Failed to allocate descriptor sets!");
		descriptorSets = descriptorSetsResult.value;
	}
}
