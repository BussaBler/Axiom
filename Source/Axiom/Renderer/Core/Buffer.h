#pragma once
#include "CommandPool.h"
#include "Device.h"
#include "Queue.h"
#include "Fence.h"

namespace Axiom {
	class Buffer {
	public:
		Buffer(uint32_t usage, uint64_t size, uint32_t memoryPropertyFlags) : usage(usage), totalSize(size), memoryPropertyFlags(memoryPropertyFlags) {}
		virtual ~Buffer() = default;

		virtual void resize(uint64_t newSize, CommandPool& commandPool, Fence* fence, Queue& queue) = 0;
		virtual void bind(uint64_t offset) = 0;
		virtual void lock(uint64_t size, uint32_t flags, uint64_t offset = 0) = 0;
		virtual void unlock() = 0;
		virtual void copyFrom(const void* data, uint64_t size, uint32_t flags, uint64_t offset = 0) = 0;
		virtual void copyTo(Buffer& destination, CommandPool& commandPool, Fence* fence, Queue& queue, uint64_t size, uint64_t srcOffset = 0, uint64_t dstOffset = 0) = 0;
		
		template<typename T>
		const T getHandle() const { return std::any_cast<T>(handle); }
		uint64_t getSize() const { return totalSize; }
		uint32_t getUsage() const { return usage; }

		static std::unique_ptr<Buffer> create(Device& deviceRef, uint32_t usage, uint64_t size, uint32_t memoryPropertyFlags, bool shouldBind);
	
	protected:
		std::any handle;
		uint64_t totalSize;
		uint32_t usage;
		bool isLocked = false;
		void* data = nullptr;
		int memoryIndex;
		uint32_t memoryPropertyFlags;
	};
}