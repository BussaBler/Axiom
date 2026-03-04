#include "VulkanBuffer.h"

namespace Axiom {
	VulkanBuffer::VulkanBuffer(Vk::Device logicDevice, Vk::PhysicalDevice physicalDevice, const CreateInfo& createInfo) : device(logicDevice), size(createInfo.size) {
		Vk::BufferCreateInfo bufferCreateInfo({}, size, AxToVkBufferUsage(createInfo.usage), Vk::SharingMode::eExclusive);
		Vk::ResultValue<Vk::Buffer> bufferResult = device.createBuffer(bufferCreateInfo);

		AX_CORE_ASSERT(bufferResult.result == Vk::Result::eSuccess, "Failed to create buffer!");
		buffer = bufferResult.value;

		Vk::MemoryRequirements memoryRequirements = device.getBufferMemoryRequirements(buffer);
		uint32_t memoryTypeIndex = findMemoryType(physicalDevice, memoryRequirements.memoryTypeBits, AxToVkMemProperty(createInfo.memoryUsage));
		Vk::MemoryAllocateInfo memoryAllocateInfo(memoryRequirements.size, memoryTypeIndex);
		Vk::ResultValue<Vk::DeviceMemory> memoryAllocResult = device.allocateMemory(memoryAllocateInfo);

		AX_CORE_ASSERT(memoryAllocResult.result == Vk::Result::eSuccess, "Failed to allocate buffer memory");
		bufferMemory = memoryAllocResult.value;

		Vk::Result result = device.bindBufferMemory(buffer, bufferMemory, 0);
		AX_CORE_ASSERT(result == Vk::Result::eSuccess, "Failed to bind buffer memory");
	}

	VulkanBuffer::~VulkanBuffer() {
		if (bufferMemory) {
			device.freeMemory(bufferMemory);
		}
		if (buffer) {
			device.destroyBuffer(buffer);
		}
	}

	uint32_t VulkanBuffer::getSize() const {
		return size;
	}

	void* VulkanBuffer::map() {
		Vk::ResultValue<void*> mapResult = device.mapMemory(bufferMemory, 0, size);

		AX_CORE_ASSERT(mapResult.result == Vk::Result::eSuccess, "Failed to map buffer memory");
		return mapResult.value;
	}

	void VulkanBuffer::unmap() {
		device.unmapMemory(bufferMemory);
	}

	uint32_t VulkanBuffer::findMemoryType(Vk::PhysicalDevice physicalDevice, uint32_t typeFilter, Vk::MemoryPropertyFlags properties) {
		Vk::PhysicalDeviceMemoryProperties memoryPropertiesResult = physicalDevice.getMemoryProperties();

		for (uint32_t i = 0; i < memoryPropertiesResult.memoryTypeCount; i++) {
			if ((typeFilter & (1 << i)) && (memoryPropertiesResult.memoryTypes[i].propertyFlags & properties) == properties) {
				return i;
			}
		}

		return 0;
	}
}
