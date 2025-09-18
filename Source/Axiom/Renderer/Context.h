#pragma once

namespace Axiom {
	class Device;
	class CommandBuffer;
	class Swapchain;

	class Context {
	public:
		virtual ~Context() = default;

		virtual bool begin(Swapchain& swapchain) { return true; }
		virtual void end(Swapchain& swapchain) {}
		virtual uint32_t getCurrentFrameIndex() const { return 0; }
		virtual void incrementFrameIndex() {}
		virtual uint32_t getFrameCount() const { return 0; }
		virtual CommandBuffer& getMainCommandBuffer() = 0;

	private:

	};
}