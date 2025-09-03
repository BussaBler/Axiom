#pragma once
#include "Platform/Vulkan/VulkanBuffer.h"
#include "Platform/Vulkan/VulkanPipeline.h"
#include "Platform/Vulkan/VulkanTexture.h"
#include "Renderer/Core/MaterialShader.h"
#include "Renderer/System/TextureSystem.h"
#include <shaderc/shaderc.hpp>

namespace Axiom {
	struct VulkanShadersStage {
		VkShaderModule handle;
		VkShaderModuleCreateInfo createInfo;
		VkPipelineShaderStageCreateInfo stageCreateInfo;
	};

	class VulkanMaterialShader : public MaterialShader {
	public:
		VulkanMaterialShader(VulkanDevice& vkDevice);
		~VulkanMaterialShader() override;

		void use(CommandBuffer& commandBuffer) override;
		void createPipeline(RenderPass& renderPass, uint32_t framebufferWidth, uint32_t framebufferHeight) override;
		void updateGlobalUniformBuffer(const GlobalUniformObject& uboData) override;
		void updateGlobalUniformBufferState(CommandBuffer& commandBuffer, uint32_t imageIndex, float deltaTime) override;
		void updateObjectUniformBufferState(CommandBuffer& commandBuffer, uint32_t imageIndex, const GeometryRenderData& data) override;
		void acquireResources(std::shared_ptr<Material> material) override;
		void releaseResources(std::shared_ptr<Material> material) override;

	private:
		void createShaderModule(const std::string& shaderName, const std::string& shaderStageStr, VkShaderStageFlagBits shaderStageFlag, uint32_t index);
		std::vector<uint32_t> getShaderByteCode(std::filesystem::path filePath, VkShaderStageFlagBits shaderStageFlag);

	public:
		static constexpr uint32_t SHADER_STAGE_COUNT = 2;
		static constexpr const char* BUILTIN_OBJECT_SHADER_NAME = "Builtin.MaterialShader";
		static constexpr uint32_t MAX_OBJECT_COUNT = 1024;
		static constexpr uint32_t SHADER_DESCRIPTOR_COUNT = 2;

	private:
		VulkanDevice& device;
		std::unique_ptr<VulkanPipeline> pipeline;
		std::vector<VulkanShadersStage> shaderStages;
		std::filesystem::path shaderDir;

		struct VulkanDescriptorState {
			std::array<uint32_t, 3> generations;
			std::array<uint32_t, 3> ids;
		};

		struct ObjectState {
			std::array<VkDescriptorSet, 3> descriptorSets;
			std::array<VulkanDescriptorState, VulkanMaterialShader::SHADER_DESCRIPTOR_COUNT> descriptorSetStates;
		};

		GlobalUniformObject globalUbo;
		VkDescriptorPool globalDescriptorPool;
		VkDescriptorSetLayout globalDescriptorSetLayout;
		std::array<VkDescriptorSet, 3> globalDescriptorSets;
		std::unique_ptr<VulkanBuffer> globalUniformBuffer;

		VkDescriptorPool objectDescriptorPool;
		VkDescriptorSetLayout objectDescriptorSetLayout;
		std::unique_ptr<VulkanBuffer> objectUniformBuffer;
		std::array<ObjectState, MAX_OBJECT_COUNT> instanceStates;
		uint32_t objectBufferIndex = 0;
		std::array<TextureUse, 1> samplerUses;
	};
}

