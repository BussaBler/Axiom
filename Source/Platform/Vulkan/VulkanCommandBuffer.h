#pragma once
#include "Core/Assert.h"
#include "Renderer/CommandBuffer.h"
#include "VulkanBuffer.h"
#include "VulkanPipeline.h"
#include "VulkanTexture.h"
#include "VulkanUtils.h"

namespace Axiom {
	class VulkanCommandBuffer : public CommandBuffer {
	public:
		VulkanCommandBuffer(Vk::Device logicDevice, Vk::CommandPool commandPool);
		~VulkanCommandBuffer() override;
		void begin() override;
		void end() override;
		void beginRendering(const RenderPass& renderPass) override;
		void endRendering() override;
		void bindPipeline(Pipeline* pipeline) override;
		void setViewport(float x, float y, float width, float height, float minDepth = 0.0f, float maxDepth = 1.0f) override;
		void setScissor(int32_t x, int32_t y, uint32_t width, uint32_t height) override;
		void bindResources() override; // TODO: parameters
		void bindVertexBuffers(const std::vector<Buffer*>& vertexBuffers) override;
		void bindIndexBuffer(Buffer* indexBuffer) override;
		void draw(uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex = 0, uint32_t firstInstance = 0) override;
		void drawIndexed(uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex = 0, int32_t vertexOffset = 0, uint32_t firstInstance = 0) override;
		void pipelineBarrier(const std::vector<Texture::Barrier>& textureBarries) override; // TODO: parameters

		inline Vk::CommandBuffer getHandle() const { return commandBuffer; }

	private:

	private:
		Vk::Device device;
		Vk::CommandPool commandPool;
		Vk::CommandBuffer commandBuffer;
	};
}

