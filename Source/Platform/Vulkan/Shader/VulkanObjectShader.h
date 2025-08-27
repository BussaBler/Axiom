#pragma once
#include "Platform/Vulkan/VulkanBuffer.h"
#include "Platform/Vulkan/VulkanTexture.h"
#include "Renderer/GeometryRenderData.h"
#include "Renderer/GlobalUniformObject.h"
#include "Renderer/LocalUniformObject.h"
#include "VulkanShader.h"

namespace Axiom {
	class VulkanObjectShader : VulkanShader {
	public:
		VulkanObjectShader(VulkanDevice& vkDevice);
		~VulkanObjectShader() override;

		void use(VulkanCommandBuffer& commandBuffer) override;
		void createPipeline(VulkanRenderPass& renderPass, uint32_t framebufferWidth, uint32_t framebufferHeight) override;
		void updateGlobalUniformBuffer(const GlobalUniformObject& uboData);
		void updateGlobalUniformBufferState(VulkanCommandBuffer& commandBuffer, uint32_t imageIndex, float deltaTime);
		void updateObjectUniformBufferState(VulkanCommandBuffer& commandBuffer, uint32_t imageIndex, const GeometryRenderData& data);
		uint32_t acquireResources();
		void releaseResources(uint32_t objectId);

	public:
		static constexpr uint32_t SHADER_STAGE_COUNT = 2;
		static constexpr const char* BUILTIN_OBJECT_SHADER_NAME = "Builtin.ObjectShader";
		static constexpr uint32_t MAX_OBJECT_COUNT = 1024;
		static constexpr uint32_t SHADER_DESCRIPTOR_COUNT = 2;

	private:
		struct VulkanDescriptorState {
			std::array<uint32_t, 3> generations;
		};

		struct ObjectState {
			std::array<VkDescriptorSet, 3> descriptorSets;
			std::array<VulkanDescriptorState, VulkanObjectShader::SHADER_DESCRIPTOR_COUNT> descriptorSetStates;
		};

		GlobalUniformObject globalUbo;
		VkDescriptorPool globalDescriptorPool;
		VkDescriptorSetLayout globalDescriptorSetLayout;
		std::array<VkDescriptorSet, 3> globalDescriptorSets;
		std::unique_ptr<VulkanBuffer> globalUniformBuffer;

		VkDescriptorPool objectDescriptorPool;
		VkDescriptorSetLayout objectDescriptorSetLayout;
		std::unique_ptr<VulkanBuffer> objectUniformBuffer;
		std::array<ObjectState, MAX_OBJECT_COUNT> objectStates;
		uint32_t objectBufferIndex = 0;
	};
}

