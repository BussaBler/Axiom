#pragma once

namespace Axiom {
	class Device;
	class CommandBuffer;

	class Context {
	public:
		virtual ~Context() = default;

		virtual void begin() {}
		virtual void end() {}
		virtual uint32_t getCurrentFrameIndex() const { return 0; }
		virtual uint32_t getFrameCount() const { return 0; }
		virtual CommandBuffer& getMainCommandBuffer() = 0;

	private:

	};
}