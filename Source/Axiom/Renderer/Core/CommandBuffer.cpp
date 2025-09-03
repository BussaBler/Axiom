#include "axpch.h"
#include "CommandBuffer.h"
#include "Platform/Vulkan/VulkanCommandBuffer.h"

namespace Axiom {
	std::unique_ptr<CommandBuffer> CommandBuffer::create(Device& deviceRef) {
		return std::make_unique<VulkanCommandBuffer>(static_cast<VulkanDevice&>(deviceRef));
	}
}
