#include "VulkanBuffer.h"
#include "VulkanDevice.h"
#include "VulkanCommandBuffer.h"

namespace Axiom {
	VulkanBuffer::~VulkanBuffer() {
		if (buffer != VK_NULL_HANDLE) {
			device.waitIdle();
			device.getHandle().destroyBuffer(buffer);
			buffer = VK_NULL_HANDLE;
		}
	}

	void VulkanBuffer::init(ResourceCreateInfo& createInfo) {
		Vk::BufferCreateInfo bufferCreateInfo({}, createInfo.size, getBufferUsage(createInfo.usage), Vk::SharingMode::eExclusive);
		Vk::ResultValue<Vk::Buffer> bufferResult = device.getHandle().createBuffer(bufferCreateInfo);

		AX_CORE_ASSERT(bufferResult.result == Vk::Result::eSuccess, "Failed to create Vulkan Buffer!");
		buffer = bufferResult.value;
	
		Vk::MemoryRequirements memRequirements = device.getHandle().getBufferMemoryRequirements(buffer);

		memorySize = memRequirements.size;
		uint32_t memoryTypeIndex = device.getAdapter().findMemoryType(memRequirements.memoryTypeBits, getMemoryPropertyFlags(createInfo.memoryUsage));
		
		Vk::MemoryAllocateInfo allocInfo(memRequirements.size, memoryTypeIndex);
		Vk::ResultValue<Vk::DeviceMemory> memoryResult = device.getHandle().allocateMemory(allocInfo);
		AX_CORE_ASSERT(memoryResult.result == Vk::Result::eSuccess, "Failed to allocate Vulkan Buffer memory!");
		memory = memoryResult.value;
		bind(0);
	}

	void VulkanBuffer::lockMemory(void*& mappedMemory, uint64_t offset, uint64_t size, uint32_t flags) {
		if (locked) {
			AX_CORE_LOG_WARN("Vulkan Buffer memory is already locked!");
			return;
		}
		if (size == VK_WHOLE_SIZE) {
			size = memorySize;
		}
		Vk::ResultValue<void*> mapResult = device.getHandle().mapMemory(memory, offset, size);
		AX_CORE_ASSERT(mapResult.result == Vk::Result::eSuccess, "Failed to map Vulkan Buffer memory!");
		mappedMemory = mapResult.value;
		locked = true;
	}

	void VulkanBuffer::unlockMemory() {
		if (!locked) {
			AX_CORE_LOG_WARN("Vulkan Buffer memory is not locked!");
			return;
		}
		device.getHandle().unmapMemory(memory);
		locked = false;
	}

	void VulkanBuffer::loadData(void* data, uint64_t size, uint64_t offset, uint32_t flags) {
		void* mappedMemory = nullptr;
		lockMemory(mappedMemory, offset, size, flags);
		memcpy(mappedMemory, data, size);
		unlockMemory();
	}

	void VulkanBuffer::copyTo(Resource& destination, uint64_t srcOffset, uint64_t dstOffset, uint64_t size) {
		VulkanQueue* queuePtr = device.getQueue(Vk::QueueFlagBits::eGraphics | Vk::QueueFlagBits::eTransfer | Vk::QueueFlagBits::eCompute);
		Vk::Queue queue = queuePtr->getHandle();
		Vk::CommandPool commandPool = queuePtr->getCommandPool();
		VulkanBuffer& destinationBuffer = static_cast<VulkanBuffer&>(destination);
		queuePtr->wait();

		VulkanCommandBuffer commandBuffer(device);
		commandBuffer.allocateAndBeginSingleUse(commandPool, true);
		Vk::DeviceSize bufferSize = (size == VK_WHOLE_SIZE) ? memorySize : size;
		Vk::BufferCopy copyRegion(srcOffset, dstOffset, bufferSize);
		commandBuffer.getHandle().copyBuffer(buffer, destinationBuffer.getHandle(), copyRegion);
		commandBuffer.endSingleUse(queue, commandPool);
	}

	void VulkanBuffer::bind(uint64_t offset) {
		AX_CORE_ASSERT(device.getHandle().bindBufferMemory(buffer, memory, offset) == Vk::Result::eSuccess, "Failed to bind Vulkan Buffer memory!");
	}

	Vk::BufferUsageFlags VulkanBuffer::getBufferUsage(uint32_t usage) {
		Vk::BufferUsageFlags bufferUsage = Vk::BufferUsageFlags();
		if (usage & ResourceUsage::VertexBuffer) {
			bufferUsage |= Vk::BufferUsageFlagBits::eVertexBuffer;
		}
		if (usage & ResourceUsage::IndexBuffer) {
			bufferUsage |= Vk::BufferUsageFlagBits::eIndexBuffer;
		}
		if (usage & ResourceUsage::UniformBuffer) {
			bufferUsage |= Vk::BufferUsageFlagBits::eUniformBuffer;
		}
		if (usage & ResourceUsage::ShaderResource) {
			bufferUsage |= Vk::BufferUsageFlagBits::eStorageBuffer;
		}
		if (usage & ResourceUsage::TransferDst) {
			bufferUsage |= Vk::BufferUsageFlagBits::eTransferDst;
		}
		if (usage & ResourceUsage::TransferSrc) {
			bufferUsage |= Vk::BufferUsageFlagBits::eTransferSrc;
		}
		if (usage & ResourceUsage::CopyDst) {
			bufferUsage |= Vk::BufferUsageFlagBits::eTransferDst;
		}
		if (usage & ResourceUsage::CopySrc) {
			bufferUsage |= Vk::BufferUsageFlagBits::eTransferSrc;
		}
		return bufferUsage;
	}
}