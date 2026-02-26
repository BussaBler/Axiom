#include "axpch.h"
#include "VulkanQueue.h"
#include "VulkanDevice.h"

namespace Axiom {
	VulkanQueue::VulkanQueue(VulkanDevice& device, uint32_t familyIndex, uint32_t queueIndex) 
		: device(device), familyIndex(familyIndex), queueIndex(queueIndex) {
		queue = device.getHandle().getQueue(familyIndex, queueIndex);

		Vk::CommandPoolCreateInfo poolInfo(Vk::CommandPoolCreateFlagBits::eResetCommandBuffer, familyIndex);
		Vk::ResultValue<Vk::CommandPool> commandPoolResult = device.getHandle().createCommandPool(poolInfo);

		AX_CORE_ASSERT(commandPoolResult.result == Vk::Result::eSuccess, "Failed to create command pool!");
		commandPool = commandPoolResult.value;
	}

	VulkanQueue::~VulkanQueue() {
		device.getHandle().destroyCommandPool(commandPool);
	}

	void VulkanQueue::wait() const {
		AX_CORE_ASSERT(queue.waitIdle() == Vk::Result::eSuccess, "Failed to wait for queue idle!");
	}
}
