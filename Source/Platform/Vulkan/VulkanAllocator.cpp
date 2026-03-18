#include "VulkanAllocator.h"

namespace Axiom {
	VulkanAllocator* VulkanAllocator::instance = nullptr;

	VulkanAllocator::VulkanAllocator(Vk::Device logicalDevice, Vk::PhysicalDevice physicalDevice)
		: device(logicalDevice), physicalDevice(physicalDevice) {
	}

	uint32_t VulkanAllocator::findMemoryType(uint32_t typeFilter, Vk::MemoryPropertyFlags properties) {
		Vk::PhysicalDeviceMemoryProperties memoryPropertiesResult = physicalDevice.getMemoryProperties();

		for (uint32_t i = 0; i < memoryPropertiesResult.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memoryPropertiesResult.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		return 0;
	}

	void VulkanAllocator::init(Vk::Device logicalDevice, Vk::PhysicalDevice physicalDevice) {
		if (instance) {
			AX_CORE_LOG_WARN("An instance of VulkanAllocator is already initialized!");
			return;
		}
		instance = new VulkanAllocator(logicalDevice, physicalDevice);
	}

	void VulkanAllocator::shutdown() {
		if (!instance) {
			AX_CORE_LOG_WARN("No instance of VulkanAllocator to shutdown!");
			return;
		}
		delete instance;
	}

	Allocation VulkanAllocator::allocate(Vk::Buffer buffer, Vk::MemoryPropertyFlags memoryProperties) {
		Vk::MemoryRequirements memoryRequirements = instance->device.getBufferMemoryRequirements(buffer);
		uint32_t memoryTypeIndex = instance->findMemoryType(memoryRequirements.memoryTypeBits, memoryProperties);

		if (instance->memoryPools.find(memoryTypeIndex) == instance->memoryPools.end()) {
			instance->memoryPools[memoryTypeIndex] = std::make_unique<VulkanMemoryPool>(instance->device, instance->DEFAULT_POOL_SIZE, memoryTypeIndex);
		}

		VulkanMemoryPool* pool = instance->memoryPools[memoryTypeIndex].get();
		Allocation alloc = pool->allocate(memoryRequirements.size, memoryRequirements.alignment);
		alloc.memoryTypeIndex = memoryTypeIndex;
		return alloc;
	}

	Allocation VulkanAllocator::allocate(Vk::Image image, Vk::MemoryPropertyFlags memoryProperties) {
		Vk::MemoryRequirements memoryRequirements = instance->device.getImageMemoryRequirements(image);
		uint32_t memoryTypeIndex = instance->findMemoryType(memoryRequirements.memoryTypeBits, memoryProperties);

		if (instance->memoryPools.find(memoryTypeIndex) == instance->memoryPools.end()) {
			instance->memoryPools[memoryTypeIndex] = std::make_unique<VulkanMemoryPool>(instance->device, instance->DEFAULT_POOL_SIZE, memoryTypeIndex);
		}

		VulkanMemoryPool* pool = instance->memoryPools[memoryTypeIndex].get();
		Allocation alloc = pool->allocate(memoryRequirements.size, memoryRequirements.alignment);
		alloc.memoryTypeIndex = memoryTypeIndex;
		return alloc;
	}

	void VulkanAllocator::free(const Allocation& allocation) {
		instance->memoryPools[allocation.memoryTypeIndex]->free(allocation);
	}
}
