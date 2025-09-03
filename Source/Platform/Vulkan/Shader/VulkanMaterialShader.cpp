#include "VulkanMaterialShader.h"

namespace Axiom {
	VulkanMaterialShader::VulkanMaterialShader(VulkanDevice& vkDevice) 
		: device(vkDevice), globalDescriptorSets(), instanceStates() {
		shaderDir = std::filesystem::current_path() / "Axiom" / "Assets" / "Shaders";
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
		AX_CORE_ASSERT(vkCreateDescriptorSetLayout(device.getHandle<VkDevice>(), &layoutInfo, nullptr, &globalDescriptorSetLayout) == VK_SUCCESS, "Failed to create global descriptor set layout!");
	
		VkDescriptorPoolSize poolSize{};
		poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		poolSize.descriptorCount = globalDescriptorSets.size();
		
		VkDescriptorPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
		poolInfo.poolSizeCount = 1;
		poolInfo.pPoolSizes = &poolSize;
		poolInfo.maxSets = globalDescriptorSets.size();
		AX_CORE_ASSERT(vkCreateDescriptorPool(device.getHandle<VkDevice>(), &poolInfo, nullptr, &globalDescriptorPool) == VK_SUCCESS, "Failed to create descriptor pool!");

		samplerUses[0] = TextureUse::DIFFUSE;

		// Local descriptors
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
		AX_CORE_ASSERT(vkCreateDescriptorSetLayout(device.getHandle<VkDevice>(), &layoutInfo, nullptr, &objectDescriptorSetLayout) == VK_SUCCESS, "Failed to create object descriptor set layout!");
	
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
		poolInfo.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
		AX_CORE_ASSERT(vkCreateDescriptorPool(device.getHandle<VkDevice>(), &poolInfo, nullptr, &objectDescriptorPool) == VK_SUCCESS, "Failed to create object descriptor pool!");
	}

	VulkanMaterialShader::~VulkanMaterialShader() {
		if (globalDescriptorSetLayout) {
			vkDestroyDescriptorSetLayout(device.getHandle<VkDevice>(), globalDescriptorSetLayout, nullptr);
			globalDescriptorSetLayout = VK_NULL_HANDLE;
		}
		if (globalDescriptorPool) {
			vkDestroyDescriptorPool(device.getHandle<VkDevice>(), globalDescriptorPool, nullptr);
			globalDescriptorPool = VK_NULL_HANDLE;
		}
		if (objectDescriptorSetLayout) {
			vkDestroyDescriptorSetLayout(device.getHandle<VkDevice>(), objectDescriptorSetLayout, nullptr);
			objectDescriptorSetLayout = VK_NULL_HANDLE;
		}
		if (objectDescriptorPool) {
			vkDestroyDescriptorPool(device.getHandle<VkDevice>(), objectDescriptorPool, nullptr);
			objectDescriptorPool = VK_NULL_HANDLE;
		}
		for (uint32_t i = 0; i < SHADER_STAGE_COUNT; i++) {
			if (shaderStages[i].handle) {
				vkDestroyShaderModule(device.getHandle<VkDevice>(), shaderStages[i].handle, nullptr);
				shaderStages[i].handle = VK_NULL_HANDLE;
			}
		}
	}

	void VulkanMaterialShader::use(CommandBuffer& commandBuffer) {
		AX_CORE_ASSERT(pipeline, "VulkanMaterialShader pipeline is not created!");
		pipeline->bind(commandBuffer, PipelineBindPoint::GRAPHICS);
	}

	void VulkanMaterialShader::createPipeline(RenderPass& renderPass, uint32_t framebufferWidth, uint32_t framebufferHeight) {
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
		AX_CORE_ASSERT(vkAllocateDescriptorSets(device.getHandle<VkDevice>(), &allocInfo, globalDescriptorSets.data()) == VK_SUCCESS, "Failed to allocate descriptor sets!");
	
		objectUniformBuffer = std::make_unique<VulkanBuffer>(device, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, sizeof(GlobalUniformObject), VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, true);
	}

	void VulkanMaterialShader::updateGlobalUniformBuffer(const GlobalUniformObject& uboData) {
		globalUbo = uboData;
		globalUniformBuffer->copyFrom(&globalUbo, sizeof(GlobalUniformObject), 0);
	}

	void VulkanMaterialShader::updateGlobalUniformBufferState(CommandBuffer& commandBuffer, uint32_t imageIndex, float deltaTime) {
		vkCmdBindDescriptorSets(commandBuffer.getHandle<VkCommandBuffer>(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getLayout(), 0, 1, &globalDescriptorSets[imageIndex], 0, nullptr);
		
		VkDescriptorBufferInfo bufferInfo{};
		bufferInfo.buffer = globalUniformBuffer->getHandle<VkBuffer>();
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

		vkUpdateDescriptorSets(device.getHandle<VkDevice>(), 1, &descriptorWrite, 0, nullptr);
	}

	void VulkanMaterialShader::updateObjectUniformBufferState(CommandBuffer& commandBuffer, uint32_t imageIndex, const GeometryRenderData& data) {
		vkCmdPushConstants(commandBuffer.getHandle<VkCommandBuffer>(), pipeline->getLayout(), VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(Math::Mat4), &data.getModelMatrix());
		
		ObjectState& state = instanceStates[data.getMaterial()->getInternalId()];
		VkDescriptorSet descriptorSet = state.descriptorSets[imageIndex];

		std::array<VkWriteDescriptorSet, SHADER_DESCRIPTOR_COUNT> descriptorWrites{};
		LocalUniformObject localUbo{};
		localUbo.setDiffuseColor(data.getMaterial()->getDiffuseColor());

		objectUniformBuffer->copyFrom(&localUbo, sizeof(LocalUniformObject), 0);

		uint32_t descriptorIndex = 0;
		uint32_t descriptorCount = 0;
		uint32_t& globalUboGeneration = state.descriptorSetStates[descriptorIndex].generations[imageIndex];
		if (globalUboGeneration == UINT32_MAX || globalUboGeneration != data.getMaterial()->getGeneration()) {
			VkDescriptorBufferInfo bufferInfo{};
			bufferInfo.buffer = objectUniformBuffer->getHandle<VkBuffer>();
			bufferInfo.offset = sizeof(LocalUniformObject) * data.getMaterial()->getInternalId();
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

			globalUboGeneration = data.getMaterial()->getGeneration();
		}
		descriptorIndex++;

		// TODO: Handle multiple textures
		const uint32_t samplers = 1;
		std::array<VkDescriptorImageInfo, samplers> imageInfos{};
		for (uint32_t i = 0; i < samplers; i++) {
			auto use = samplerUses[i];
			std::shared_ptr<Texture> texture = nullptr;
			switch (use)	
			{
				case Axiom::TextureUse::DIFFUSE:
					texture = data.getMaterial()->getDiffuseMap().texture;
					break;
				default:
					AX_CORE_LOG_ERROR("Unsupported texture use!");
					return;
			}

			uint32_t descGeneration = state.descriptorSetStates[descriptorIndex].generations[imageIndex];
			uint32_t descId = state.descriptorSetStates[descriptorIndex].ids[imageIndex];

			if (texture->getGeneration() == UINT32_MAX) {
				texture = TextureSystem::getDefaultTexture();
				descGeneration = UINT32_MAX;
			}

			if (texture && (descId != texture->getId() || descGeneration != texture->getGeneration() || descGeneration == UINT32_MAX)) {
				auto vkTexture = std::static_pointer_cast<VulkanTexture>(texture);

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
			vkUpdateDescriptorSets(device.getHandle<VkDevice>(), descriptorCount, descriptorWrites.data(), 0, nullptr);
		}
		vkCmdBindDescriptorSets(commandBuffer.getHandle<VkCommandBuffer>(), VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->getLayout(), 1, 1, &descriptorSet, 0, nullptr);
	}

	void VulkanMaterialShader::acquireResources(std::shared_ptr<Material> material) {
		material->setInternalId(objectBufferIndex);
		objectBufferIndex++;

		ObjectState& state = instanceStates[material->getInternalId()];
		for (uint32_t i = 0; i < SHADER_DESCRIPTOR_COUNT; i++) {
			state.descriptorSetStates[i].generations.fill(UINT32_MAX);
			state.descriptorSetStates[i].ids.fill(UINT32_MAX);
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
		AX_CORE_ASSERT(vkAllocateDescriptorSets(device.getHandle<VkDevice>(), &allocInfo, state.descriptorSets.data()) == VK_SUCCESS, "Failed to allocate object descriptor sets!");
	}

	void VulkanMaterialShader::releaseResources(std::shared_ptr<Material> material) {
		ObjectState& state = instanceStates[material->getInternalId()];
		vkFreeDescriptorSets(device.getHandle<VkDevice>(), objectDescriptorPool, static_cast<uint32_t>(state.descriptorSets.size()), state.descriptorSets.data());
	
		for (uint32_t i = 0; i < SHADER_DESCRIPTOR_COUNT; i++) {
			state.descriptorSetStates[i].generations.fill(UINT32_MAX);
			state.descriptorSetStates[i].ids.fill(UINT32_MAX);
		}

		material->setInternalId(UINT32_MAX);
	}

	void VulkanMaterialShader::createShaderModule(const std::string& shaderName, const std::string& shaderStageStr, VkShaderStageFlagBits shaderStageFlag, uint32_t index) {
		std::string shaderFileName = shaderName + "." + shaderStageStr;

		auto shaderCode = getShaderByteCode(shaderDir / shaderFileName, shaderStageFlag);

		shaderStages[index].createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		shaderStages[index].createInfo.codeSize = shaderCode.size() * sizeof(uint32_t);
		shaderStages[index].createInfo.pCode = shaderCode.data();

		AX_CORE_ASSERT(vkCreateShaderModule(device.getHandle<VkDevice>(), &shaderStages[index].createInfo, nullptr, &shaderStages[index].handle) == VK_SUCCESS,
			"Failed to create shader module for shader: {0}", shaderFileName);

		shaderStages[index].stageCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStages[index].stageCreateInfo.stage = shaderStageFlag;
		shaderStages[index].stageCreateInfo.module = shaderStages[index].handle;
		shaderStages[index].stageCreateInfo.pName = "main"; // Entry point name
	}

	std::vector<uint32_t> VulkanMaterialShader::getShaderByteCode(std::filesystem::path filePath, VkShaderStageFlagBits shaderStageFlag) {
		std::string sourceCode = FileSystem::readFileStr(filePath);
		shaderc::Compiler compiler;

		shaderc_shader_kind kind;
		switch (shaderStageFlag) {
		case VK_SHADER_STAGE_VERTEX_BIT:
			kind = shaderc_glsl_default_vertex_shader;
			break;
		case VK_SHADER_STAGE_FRAGMENT_BIT:
			kind = shaderc_glsl_default_fragment_shader;
			break;
		default:
			kind = shaderc_glsl_default_vertex_shader;
		}
		auto result = compiler.CompileGlslToSpv(sourceCode, kind, filePath.string().c_str());

		if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
			AX_CORE_LOG_ERROR("Shader compilation failed: {0}", result.GetErrorMessage());
			return {};
		}

		return { result.begin(), result.end() };
	}
}
