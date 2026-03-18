#pragma once
#include "Core/Assert.h"
#include "VulkanInclude.h"

namespace Axiom {
	struct MemoryChunk {
		size_t offset;
		size_t size;
		bool isFree;
	};

	struct Allocation {
		Vk::DeviceMemory memory;
		size_t offset;
		uint32_t memoryTypeIndex;
		std::list<MemoryChunk>::iterator chunkIt;
	};

	class VulkanMemoryPool {
	public:
		VulkanMemoryPool(Vk::Device logicalDevice, size_t size, uint32_t memoryTypeIndex);
		~VulkanMemoryPool();

		Allocation allocate(size_t size, size_t alignment);
		void free(const Allocation& allocation);

	private:
		Vk::Device device = nullptr;
		Vk::DeviceMemory memory = nullptr;
		size_t poolSize = 0;
		std::list<MemoryChunk> chunks;
	};
}

