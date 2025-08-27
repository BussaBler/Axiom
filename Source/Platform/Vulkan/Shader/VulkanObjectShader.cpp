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
		AX_CORE_ASSERT(vkCreateDescriptorSetLayout(device.getHandle(), &layoutInfo, nullptr, &globalDescriptorSetLayout) == VK_SUCCESS, "Failed to create global descriptor set layout!");
	
		VkDescriptorPoolSize poolSize{};
		poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSize.descriptorCount = globalDescriptorSets.size();
		
		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = 1;
		poolInfo.pPoolSizes = &poolSize;
		poolInfo.maxSets = globalDescriptorSets.size();
		AX_CORE_ASSERT(vkCreateDescriptorPool(device.getHandle(), &poolInfo, nullptr, &globalDescriptorPool) == VK_SUCCESS, "Failed to create descriptor pool!");
	
		std::array<VkDescriptorType, SHADER_DESCRIPTOR_COUNT> descriptorTypes = {
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
		};
		std::array<VkDescriptorSetLayoutBinding, SHADER_DESCRIPTOR_COUNT> objectUniformBindings{};
		for (uint32_t i = 0; i < SHADER_DESCRIPTOR_COUNT; i++) {
			objectUniformBindings[i].binding = i;
			objectUniformBindings[i].descriptorType = descriptorTypes[i];
			objectUniformBindings[i].descriptorCount = 1;
			objectUniformBindings[i].pImmutableSamplers = nullptr;
			objectUniformBindings[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
		}

		layoutInfo = {};
		layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		layoutInfo.bindingCount = static_cast<uint32_t>(objectUniformBindings.size());
		layoutInfo.pBindings = objectUniformBindings.data();
		AX_CORE_ASSERT(vkCreateDescriptorSetLayout(device.getHandle(), &layoutInfo, nullptr, &objectDescriptorSetLayout) == VK_SUCCESS, "Failed to create object descriptor set layout!");
	
		std::array<VkDescriptorPoolSize, 2> objectPoolSizes{};
		objectPoolSizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		objectPoolSizes[0].descriptorCount = MAX_OBJECT_COUNT;
		objectPoolSizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
		objectPoolSizes[1].descriptorCount = MAX_OBJECT_COUNT;

		poolInfo = {};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = static_cast<uint32_t>(objectPoolSizes.size());
		poolInfo.pPoolSizes = objectPoolSizes.data();
		poolInfo.maxSets = MAX_OBJECT_COUNT;
		AX_CORE_ASSERT(vkCreateDescriptorPool(device.getHandle(), &poolInfo, nullptr, &objectDescriptorPool) == VK_SUCCESS, "Failed to create object descriptor pool!");
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
		if (objectDescriptorSetLayout) {
			vkDestroyDescriptorSetLayout(device.getHandle(), objectDescriptorSetLayout, nullptr);
			objectDescriptorSetLayout = VK_NULL_HANDLE;
		}
		if (objectDescriptorPool) {
			vkDestroyDescriptorPool(device.getHandle(), objectDescriptorPool, nullptr);
			objectDescriptorPool = VK_NULL_HANDLE;
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

		// Vertex input: positions (vec3) and texcoords (vec2)
		pipelineConfig.attributeDescriptions.resize(2);
		pipelineConfig.attributeDescriptions[0].binding = 0;
		pipelineConfig.attributeDescriptions[0].location = 0;
		pipelineConfig.attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT; // Vec3
		pipelineConfig.attributeDescriptions[0].offset = offsetof(Vertex, position);
		pipelineConfig.attributeDescriptions[1].binding = 0;
		pipelineConfig.attributeDescriptions[1].location = 1;
		pipelineConfig.attributeDescriptions[1].format = VK_FORMAT_R32G32_SFLOAT; // Vec2
		pipelineConfig.attributeDescriptions[1].offset = offsetof(Vertex, texCoord);

		pipelineConfig.descriptorSetLayouts.resize(2);
		pipelineConfig.descriptorSetLayouts[0] = globalDescriptorSetLayout;
		pipelineConfig.descriptorSetLayouts[1] = objectDescriptorSetLayout;

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
	
		objectUniformBuffer = std::make_unique<VulkanBuffer>(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(GlobalUniformObject), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, true);
	}

	void VulkanObjectShader::updateGlobalUniformBuffer(const GlobalUniformObject& uboData) {
		globalUbo = uboData;
		globalUniformBuffer->copyFrom(&globalUbo, sizeof(GlobalUniformObject), 0);
	}

	void VulkanObjectShader::updateGlobalUniformBufferState(VulkanCommandBuffer& commandBuffer, uint32_t imageIndex, float deltaTime) {
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

	void VulkanObjectShader::updateObjectUniformBufferState(VulkanCommandBuffer& commandBuffer, uint32_t imageIndex, const GeometryRenderData& data) {
		vkCmdPushConstants(commandBuffer.getHandle(), pipeline->getLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Math::Mat4), &data.getModelMatrix());
		
		ObjectState& state = objectStates[data.getObjectId()];
		VkDescriptorSet descriptorSet = state.descriptorSets[imageIndex];

		std::array<VkWriteDescriptorSet, SHADER_DESCRIPTOR_COUNT> descriptorWrites{};
		LocalUniformObject localUbo{};
		localUbo.setDiffuseColor(Math::Vec4::one());

		objectUniformBuffer->copyFrom(&localUbo, sizeof(LocalUniformObject), 0);

		uint32_t descriptorIndex = 0;
		uint32_t descriptorCount = 0;
		if (state.descriptorSetStates[descriptorIndex].generations[imageIndex] == UINT32_MAX) {
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = objectUniformBuffer->getHandle();
			bufferInfo.offset = sizeof(LocalUniformObject) * data.getObjectId();
			bufferInfo.range = sizeof(LocalUniformObject);

			VkWriteDescriptorSet descriptorWrite{};
			descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			descriptorWrite.dstSet = descriptorSet;
			descriptorWrite.dstBinding = 0;
			descriptorWrite.dstArrayElement = 0;
			descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			descriptorWrite.descriptorCount = 1;
			descriptorWrite.pBufferInfo = &bufferInfo;
			descriptorWrites[descriptorCount] = descriptorWrite;
			descriptorCount++;

			state.descriptorSetStates[descriptorIndex].generations[imageIndex] = 1;
		}
		descriptorIndex++;

		// TODO: Handle multiple textures
		const uint32_t samplers = 1;
		std::array<VkDescriptorImageInfo, samplers> imageInfos{};
		for (uint32_t i = 0; i < samplers; i++) {
			Texture* texture = data.getTexture(i);
			uint32_t descGeneration = state.descriptorSetStates[descriptorIndex].generations[imageIndex];

			if (texture && (descGeneration != texture->getGeneration() || descGeneration == UINT32_MAX)) {
				VulkanTexture* vkTexture = static_cast<VulkanTexture*>(texture);

				imageInfos[i].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
				imageInfos[i].imageView = vkTexture->getImageView();
				imageInfos[i].sampler = vkTexture->getSampler();

				VkWriteDescriptorSet descriptorWrite{};
				descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
				descriptorWrite.dstSet = descriptorSet;
				descriptorWrite.dstBinding = 1;
				descriptorWrite.dstArrayElement = 0;
				descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
				descriptorWrite.descriptorCount = 1;
				descriptorWrite.pImageInfo = &imageInfos[i];
				descriptorWrites[descriptorCount] = descriptorWrite;
				descriptorCount++;

				if (texture->getGeneration() != UINT32_MAX) {
					state.descriptorSetStates[descriptorIndex].generations[imageIndex] = texture->getGeneration();
				}
				descriptorIndex++;
			}
		}

		if (descriptorCount > 0) {
			vkUpdateDescriptorSets(device.getHandle(), descriptorCount, descriptorWrites.data(), 0, nullptr);
		}
		vkCmdBindDescriptorSets(commandBuffer.getHandle(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getLayout(), 1, 1, &descriptorSet, 0, nullptr);
	}

	uint32_t VulkanObjectShader::acquireResources() {
		uint32_t outId = objectBufferIndex;
		objectBufferIndex++;

		ObjectState& state = objectStates[outId];
		for (uint32_t i = 0; i < SHADER_DESCRIPTOR_COUNT; i++) {
			state.descriptorSetStates[i].generations.fill(UINT32_MAX);
		}

		std::array<VkDescriptorSetLayout, 3> layouts{
			objectDescriptorSetLayout,
			objectDescriptorSetLayout,
			objectDescriptorSetLayout
		};

		VkDescriptorSetAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		allocInfo.descriptorPool = objectDescriptorPool;
		allocInfo.descriptorSetCount = static_cast<uint32_t>(state.descriptorSets.size());
		allocInfo.pSetLayouts = layouts.data();
		AX_CORE_ASSERT(vkAllocateDescriptorSets(device.getHandle(), &allocInfo, state.descriptorSets.data()) == VK_SUCCESS, "Failed to allocate object descriptor sets!");

		return outId;
	}

	void VulkanObjectShader::releaseResources(uint32_t objectId) {
		ObjectState& state = objectStates[objectId];
		vkFreeDescriptorSets(device.getHandle(), objectDescriptorPool, static_cast<uint32_t>(state.descriptorSets.size()), state.descriptorSets.data());
	
		for (uint32_t i = 0; i < SHADER_DESCRIPTOR_COUNT; i++) {
			state.descriptorSetStates[i].generations.fill(UINT32_MAX);
		}
	}
}
