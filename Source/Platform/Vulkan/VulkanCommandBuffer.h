#pragma once
#include "Renderer/CommandBuffer.h"
#include "Core/Assert.h"
#include "VulkanUtils.h"
#include "VulkanTexture.h"

namespace Axiom {
	class VulkanCommandBuffer : public CommandBuffer {
	public:
		VulkanCommandBuffer(Vk::Device logicDevice, Vk::CommandPool commandPool);
		~VulkanCommandBuffer() override;
		void begin() override;
		void end() override;
		void beginRendering(const RenderPass& renderPass) override;
		void endRendering() override;
		void bindPipeline() override; // TODO: parameters
		void bindResources() override; // TODO: parameters
		void bindVertexBuffers() override; // TODO: parameters
		void bindIndexBuffer() override; // TODO: parameters
		void draw() override; // TODO: parameters
		void drawIndexed() override; // TODO: parameters
		void pipelineBarrier(const std::vector<Texture::Barrier>& textureBarries) override; // TODO: parameters

		inline Vk::CommandBuffer getHandle() const { return commandBuffer; }

	private:

	private:
		Vk::Device device;
		Vk::CommandPool commandPool;
		Vk::CommandBuffer commandBuffer;
	};
}

