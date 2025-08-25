#include "VulkanObjectShader.h"

namespace Axiom {
	VulkanObjectShader::VulkanObjectShader(VulkanDevice& vkDevice) : VulkanShader(vkDevice), globalDescriptorSets() {
		std::array<std::string, SHADER_STAGE_COUNT> shaderStageStr = { "vert", "frag" };
		std::array<VkShaderStageFlagBits, SHADER_STAGE_COUNT> shaderStagesFlags = { VK_SHADER_STAGE_VERTEX_BIT, VK_SHADER_STAGE_FRAGMENT_BIT };
		shaderStages.resize(SHADER_STAGE_COUNT, {});

		for (uint32_t i = 0; i < SHADER_STAGE_COUNT; i++) {
			createShaderModule(BUILTIN_OBJECT_SHADER_NAME, shaderStageStr[i], shaderStagesFlags[i], i);
		}

		VkDescriptorSetLayoutBinding uboLayoutBinding{};
		uboLayoutBinding.binding = 0;
		uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		uboLayoutBinding.descriptorCount = 1;
		uboLayoutBinding.pImmutableSamplers = nullptr;
		uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;

		VkDescriptorSetLayoutCreateInfo layoutInfo{};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = 1;
		layoutInfo.pBindings = &uboLayoutBinding;
		AX_CORE_ASSERT(vkCreateDescriptorSetLayout(device.getHandle(), &layoutInfo, nullptr, &globalDescriptorSetLayout) == VK_SUCCESS, "Failed to create descriptor set layout!");
	
		VkDescriptorPoolSize poolSize{};
		poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSize.descriptorCount = globalDescriptorSets.size();
		
		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = 1;
		poolInfo.pPoolSizes = &poolSize;
		poolInfo.maxSets = globalDescriptorSets.size();
		AX_CORE_ASSERT(vkCreateDescriptorPool(device.getHandle(), &poolInfo, nullptr, &globalDescriptorPool) == VK_SUCCESS, "Failed to create descriptor pool!");
	}

	VulkanObjectShader::~VulkanObjectShader() {
		if (globalDescriptorSetLayout) {
			vkDestroyDescriptorSetLayout(device.getHandle(), globalDescriptorSetLayout, nullptr);
			globalDescriptorSetLayout = VK_NULL_HANDLE;
		}
		if (globalDescriptorPool) {
			vkDestroyDescriptorPool(device.getHandle(), globalDescriptorPool, nullptr);
			globalDescriptorPool = VK_NULL_HANDLE;
		}
	}

	void VulkanObjectShader::use(VulkanCommandBuffer& commandBuffer) {
		AX_CORE_ASSERT(pipeline, "VulkanObjectShader pipeline is not created!");
		pipeline->bind(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS);
	}

	void VulkanObjectShader::createPipeline(VulkanRenderPass& renderPass, uint32_t framebufferWidth, uint32_t framebufferHeight) {
		PipelineConfigInfo pipelineConfig{};
		pipelineConfig.viewport.x = 0.0f;
		pipelineConfig.viewport.y = static_cast<float>(framebufferHeight);
		pipelineConfig.viewport.width = static_cast<float>(framebufferWidth);
		pipelineConfig.viewport.height = -static_cast<float>(framebufferHeight);
		pipelineConfig.viewport.minDepth = 0.0f;
		pipelineConfig.viewport.maxDepth = 1.0f;
		pipelineConfig.scissor.offset = { 0, 0 };
		pipelineConfig.scissor.extent = { framebufferWidth, framebufferHeight };

		pipelineConfig.attributeDescriptions.resize(1);
		pipelineConfig.attributeDescriptions[0].binding = 0;
		pipelineConfig.attributeDescriptions[0].location = 0;
		pipelineConfig.attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT; // Vec3
		pipelineConfig.attributeDescriptions[0].offset = 0;

		pipelineConfig.descriptorSetLayouts.resize(1);
		pipelineConfig.descriptorSetLayouts[0] = globalDescriptorSetLayout;

		pipelineConfig.pushConstantsRange.resize(1);
		pipelineConfig.pushConstantsRange[0].stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
		pipelineConfig.pushConstantsRange[0].offset = 0;
		pipelineConfig.pushConstantsRange[0].size = sizeof(Math::Mat4);

		pipelineConfig.shaderStages.resize(SHADER_STAGE_COUNT);
		for (uint32_t i = 0; i < SHADER_STAGE_COUNT; i++) {
			pipelineConfig.shaderStages[i] = shaderStages[i].stageCreateInfo;
		}

		pipeline = std::make_unique<VulkanPipeline>(device, renderPass, pipelineConfig);

		globalUniformBuffer = std::make_unique<VulkanBuffer>(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(GlobalUniformObject), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, true);
	
		std::array<VkDescriptorSetLayout, 3> layouts{
			globalDescriptorSetLayout,
			globalDescriptorSetLayout,
			globalDescriptorSetLayout
		};

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = globalDescriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(globalDescriptorSets.size());
		allocInfo.pSetLayouts = layouts.data();
		AX_CORE_ASSERT(vkAllocateDescriptorSets(device.getHandle(), &allocInfo, globalDescriptorSets.data()) == VK_SUCCESS, "Failed to allocate descriptor sets!");
	}

	void VulkanObjectShader::updateGlobalUniformBuffer(const GlobalUniformObject& uboData) {
		globalUbo = uboData;
		globalUniformBuffer->copyFrom(&globalUbo, sizeof(GlobalUniformObject), 0);
	}

	void VulkanObjectShader::updateGlobalUniformBufferState(VulkanCommandBuffer& commandBuffer, uint32_t imageIndex) {
		vkCmdBindDescriptorSets(commandBuffer.getHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getLayout(), 0, 1, &globalDescriptorSets[imageIndex], 0, nullptr);
		
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = globalUniformBuffer->getHandle();
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(GlobalUniformObject);

		VkWriteDescriptorSet descriptorWrite{};
		descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		descriptorWrite.dstSet = globalDescriptorSets[imageIndex];
		descriptorWrite.dstBinding = 0;
		descriptorWrite.dstArrayElement = 0;
		descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		descriptorWrite.descriptorCount = 1;
		descriptorWrite.pBufferInfo = &bufferInfo;

		vkUpdateDescriptorSets(device.getHandle(), 1, &descriptorWrite, 0, nullptr);
	}

	void VulkanObjectShader::updatePushConstants(VulkanCommandBuffer& commandBuffer, const Math::Mat4& model) {
		vkCmdPushConstants(commandBuffer.getHandle(), pipeline->getLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Math::Mat4), &model);
	}
}
