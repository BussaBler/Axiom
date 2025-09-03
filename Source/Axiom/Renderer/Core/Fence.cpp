#include "axpch.h"
#include "Fence.h"
#include "Platform/Vulkan/VulkanFence.h"

namespace Axiom {
	std::unique_ptr<Fence> Fence::create(Device& deviceRef, bool signaled) {
		return std::make_unique<VulkanFence>(static_cast<VulkanDevice&>(deviceRef), signaled);
	}
}
