#include "axpch.h"

#include "VulkanBuffer.h"

namespace Axiom {
    VulkanBuffer::VulkanBuffer(Vk::Device logicalDevice, const CreateInfo& createInfo) : device(logicalDevice), size(createInfo.size) {
        Vk::BufferCreateInfo bufferCreateInfo({}, size, axToVkBufferUsage(createInfo.usage), Vk::SharingMode::eExclusive);
        Vk::ResultValue<Vk::Buffer> bufferResult = device.createBuffer(bufferCreateInfo);

        AX_CORE_ASSERT(bufferResult.result == Vk::Result::eSuccess, "Failed to create buffer!");
        buffer = bufferResult.value;

        allocation = VulkanAllocator::allocate(buffer, axToVkMemProperty(createInfo.memoryUsage));

        Vk::Result result = device.bindBufferMemory(buffer, allocation.memory, allocation.offset);
        AX_CORE_ASSERT(result == Vk::Result::eSuccess, "Failed to bind buffer memory");
    }

    VulkanBuffer::~VulkanBuffer() {
        VulkanAllocator::free(allocation);
        if (buffer) {
            device.destroyBuffer(buffer);
        }
    }

    uint32_t VulkanBuffer::getSize() const {
        return size;
    }

    void VulkanBuffer::setData(const void* data, uint64_t size, uint64_t offset) {
        AX_CORE_ASSERT(allocation.mappedPtr, "setData called on a buffer that isn't CPU-visible");
        AX_CORE_ASSERT(offset + size <= this->size, "Data size exceeds buffer size");
        std::memcpy(allocation.mappedPtr + offset, data, size);
    }
} // namespace Axiom
