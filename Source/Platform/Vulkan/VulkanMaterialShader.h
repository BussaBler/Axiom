#pragma once
#include "VulkanShader.h"
#include "Renderer/GlobalUbo.h"
#include "Renderer/MaterialUbo.h"
#include "VulkanInclude.h"

namespace Axiom {
	class VulkanMaterialShader : public VulkanShader {
	public:
		VulkanMaterialShader(VulkanDevice& vkDevice) : VulkanShader(vkDevice, "Builtin.MaterialShader") {}
		~VulkanMaterialShader() override;

		void updateFrameIndex(uint32_t frameIndex) override { currentFrameIndex = frameIndex; }
		void bindDescriptors(CommandBuffer& commandBuffer) override;
		void bindUniformBuffer(Resource& uniformBuffer) override;
		void bindPushConstants(CommandBuffer& commandBuffer, const void* data, uint32_t size, uint32_t offset = 0) const override;
		void bindTexture(Texture& texture) override;

	private:
		void createPipeline(Vk::RenderPass vkRenderPass) override;
		void createDescriptors() override;

	private:
		Vk::DescriptorSetLayout descriptorSetLayout = nullptr;
		std::vector<Vk::DescriptorSet> descriptorSets;
		Vk::DescriptorPool descriptorPool = nullptr;
		uint32_t currentFrameIndex = 0;
	};
}

