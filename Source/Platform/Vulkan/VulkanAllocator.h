#pragma once
#include "VulkanInclude.h"
#include "VulkanMemoryPool.h"

namespace Axiom {
	class VulkanAllocator {
	public:
		static void init(Vk::Device logicalDevice, Vk::PhysicalDevice physicalDevice);
		static void shutdown();

		static Allocation allocate(Vk::Buffer buffer, Vk::MemoryPropertyFlags memoryProperties);
		static Allocation allocate(Vk::Image image, Vk::MemoryPropertyFlags memoryProperties);

		static void free(const Allocation& allocation);

	private:
		VulkanAllocator(Vk::Device logicalDevice, Vk::PhysicalDevice physicalDevice);
		~VulkanAllocator() = default;

		uint32_t findMemoryType(uint32_t typeFilter, Vk::MemoryPropertyFlags properties);

	private:
		const size_t DEFAULT_POOL_SIZE = 256 * 1e6;

		static VulkanAllocator* instance;
		Vk::Device device = nullptr;
		Vk::PhysicalDevice physicalDevice = nullptr;
		std::unordered_map<uint32_t, std::unique_ptr<VulkanMemoryPool>> memoryPools;
	};
}

