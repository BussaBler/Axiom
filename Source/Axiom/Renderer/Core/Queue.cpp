#include "axpch.h"
#include "Queue.h"
#include "Platform/Vulkan/VulkanQueue.h"

namespace Axiom {
	std::unique_ptr<Queue> Queue::create(Device& deviceRef, uint32_t queueIndex) {
		return std::make_unique<VulkanQueue>(static_cast<VulkanDevice&>(deviceRef), queueIndex);
	}
}
