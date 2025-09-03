#include "axpch.h"
#include "Buffer.h"
#include "Platform/Vulkan/VulkanBuffer.h"

namespace Axiom {
	std::unique_ptr<Buffer> Buffer::create(Device& deviceRef, uint32_t usage, uint64_t size, uint32_t memoryPropertyFlags, bool shouldBind) {
		return std::make_unique<VulkanBuffer>(static_cast<VulkanDevice&>(deviceRef), static_cast<VkBufferUsageFlags>(usage), size, memoryPropertyFlags, shouldBind);
	}
}
