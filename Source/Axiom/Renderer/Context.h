#pragma once

namespace Axiom {
	class Device;
	class CommandBuffer;
	class Swapchain;
	class Resource;

	class Context {
	public:
		virtual ~Context() = default;

		virtual bool begin(Swapchain& swapchain) { return true; }
		virtual void end(Swapchain& swapchain) {}
		virtual uint32_t getCurrentFrameIndex() const { return 0; }
		virtual void incrementFrameIndex() {}
		virtual uint32_t getFrameCount() const { return 0; }
		virtual CommandBuffer& getMainCommandBuffer() = 0;
		virtual void bindVertexBuffer(Resource& vertexBuffer, CommandBuffer& commandBuffer) {}
		virtual void bindIndexBuffer(Resource& indexBuffer, CommandBuffer& commandBuffer) {}
		virtual void drawIndexed(uint32_t indexCount, uint32_t instanceCount, CommandBuffer& commandBuffer) {}

	private:

	};
}