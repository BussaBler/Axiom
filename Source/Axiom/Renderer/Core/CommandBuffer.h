#pragma once
#include "CommandPool.h"
#include "Device.h"
#include "Queue.h"

namespace Axiom {
	class CommandBuffer {
	public:
		CommandBuffer() = default;
		virtual ~CommandBuffer() = default;

		virtual void allocate(CommandPool& commandPool, bool primary = true) = 0;
		virtual void free(CommandPool& commandPool) = 0;
		virtual void begin(uint32_t usageFlags = 0) = 0;
		virtual void end() = 0;
		virtual void reset() = 0;
		virtual void updateSubmitted() = 0;
		// Convenience method to allocate and begin recording in one step
		virtual void allocateAndBeginSingleUse(CommandPool& commandPool, bool primary = true) = 0;
		// Ends recording, submits and waits for the queue to become idle, and frees the command buffer
		virtual void endAndFreeSingleUse(CommandPool& commandPool, Queue& queue) = 0;

		template<typename T>
		const T getHandle() const { return std::any_cast<T>(handle); }

		static std::unique_ptr<CommandBuffer> create(Device& deviceRef);

	protected:
		std::any handle;
	};
}

