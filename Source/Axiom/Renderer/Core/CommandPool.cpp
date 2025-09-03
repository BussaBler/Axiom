#include "axpch.h"
#include "CommandPool.h"
#include "Platform/Vulkan/VulkanCommandPool.h"

namespace Axiom {
	std::unique_ptr<CommandPool> CommandPool::create(Device& deviceRef, uint32_t queueFamilyIndex) {
		return std::make_unique<VulkanCommandPool>(static_cast<VulkanDevice&>(deviceRef), queueFamilyIndex);
	}
}
