#include "axpch.h"
#include "Semaphore.h"
#include "Platform/Vulkan/VulkanSemaphore.h"

namespace Axiom {
	std::unique_ptr<Semaphore> Semaphore::create(Device& deviceRef) {
		return std::make_unique<VulkanSemaphore>(static_cast<VulkanDevice&>(deviceRef));
	}
}
