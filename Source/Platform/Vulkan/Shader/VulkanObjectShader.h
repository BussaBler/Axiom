#pragma once
#include "Platform/Vulkan/VulkanBuffer.h"
#include "Renderer/GlobalUniformObject.h"
#include "VulkanShader.h"

namespace Axiom {
	class VulkanObjectShader : VulkanShader {
	public:
		VulkanObjectShader(VulkanDevice& vkDevice);
		~VulkanObjectShader() override;

		void use(VulkanCommandBuffer& commandBuffer) override;
		void createPipeline(VulkanRenderPass& renderPass, uint32_t framebufferWidth, uint32_t framebufferHeight) override;
		void updateGlobalUniformBuffer(const GlobalUniformObject& uboData);
		void updateGlobalUniformBufferState(VulkanCommandBuffer& commandBuffer, uint32_t imageIndex);
		void updatePushConstants(VulkanCommandBuffer& commandBuffer, const Math::Mat4& model);

	private:
		GlobalUniformObject globalUbo;
		VkDescriptorPool globalDescriptorPool;
		VkDescriptorSetLayout globalDescriptorSetLayout;
		std::array<VkDescriptorSet, 3> globalDescriptorSets;
		std::unique_ptr<VulkanBuffer> globalUniformBuffer;

		static constexpr uint32_t SHADER_STAGE_COUNT = 2;
		static constexpr const char* BUILTIN_OBJECT_SHADER_NAME = "Builtin.ObjectShader";
	};
}

