#include "VulkanMemoryPool.h"

namespace Axiom {
	VulkanMemoryPool::VulkanMemoryPool(Vk::Device logicalDevice, size_t size, uint32_t memoryTypeIndex) : device(logicalDevice), poolSize(size) {
		AX_CORE_LOG_DEBUG("Creating Vulkan memory pool with size {} bytes and memory type index {}.", size, memoryTypeIndex);
		Vk::MemoryAllocateInfo allocInfo(size, memoryTypeIndex);
		Vk::ResultValue<Vk::DeviceMemory> allocResult = device.allocateMemory(allocInfo);

		AX_CORE_ASSERT(allocResult.result == Vk::Result::eSuccess, "Failed to allocate memory for memory pool");
		memory = allocResult.value;

		chunks.push_back({ 0, size, true });
	}

	VulkanMemoryPool::~VulkanMemoryPool() {
		if (memory) {
			device.freeMemory(memory);
		}
	}

	Allocation VulkanMemoryPool::allocate(size_t size, size_t alignment) {
		for (auto chunkIt = chunks.begin(); chunkIt != chunks.end(); chunkIt++) {
			size_t alignedOffset = (chunkIt->offset + alignment - 1) & ~(alignment - 1);
			size_t padding = alignedOffset - chunkIt->offset;
			size_t totalSize = padding + size;

			if (chunkIt->isFree && totalSize <= chunkIt->size) {
				size_t remainingSize = chunkIt->size - totalSize;
				Allocation allocation{};
				allocation.memory = memory;
				allocation.offset = alignedOffset;
				allocation.chunkIt = chunkIt;

				chunkIt->isFree = false;
				chunkIt->size = totalSize;

				if (remainingSize > 0) {
					MemoryChunk newChunk{};
					newChunk.offset = alignedOffset + size;
					newChunk.size = remainingSize;
					newChunk.isFree = true;
					chunks.insert(std::next(chunkIt), newChunk);
				}

				return allocation;
			}
		}

		AX_CORE_LOG_WARN("Memory pool is out of memory. Failed to allocate {} bytes with alignment {}.", size, alignment);
		return Allocation{ nullptr, 0, 0, chunks.end() };
	}

	void VulkanMemoryPool::free(const Allocation& allocation) {
		auto chunkIt = allocation.chunkIt;

		chunkIt->isFree = true;
		auto nextIt = std::next(chunkIt);
		if (nextIt != chunks.end() && nextIt->isFree) {
			chunkIt->size += nextIt->size;
			chunks.erase(nextIt);
		}

		if (chunkIt != chunks.begin()) {
			auto prevIt = std::prev(chunkIt);
			if (prevIt->isFree) {
				prevIt->size += chunkIt->size;
				chunks.erase(chunkIt);
			}
		}
	}
}
