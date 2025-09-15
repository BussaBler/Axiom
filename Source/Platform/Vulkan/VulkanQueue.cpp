#include "axpch.h"
#include "VulkanQueue.h"
#include "VulkanDevice.h"

namespace Axiom {
	VulkanQueue::VulkanQueue(VulkanDevice& device, uint32_t familyIndex, uint32_t queueIndex) 
		: device(device), familyIndex(familyIndex), queueIndex(queueIndex) {
		vkGetDeviceQueue(device.getHandle(), familyIndex, queueIndex, &queue);
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		poolInfo.queueFamilyIndex = familyIndex;
		AX_CORE_ASSERT(vkCreateCommandPool(device.getHandle(), &poolInfo, nullptr, &commandPool) == VK_SUCCESS, "Failed to create command pool!");
	}

	VulkanQueue::~VulkanQueue() {
		vkDestroyCommandPool(device.getHandle(), commandPool, nullptr);
	}

	void VulkanQueue::wait() const {
		AX_CORE_ASSERT(vkQueueWaitIdle(queue) == VK_SUCCESS, "Failed to wait for queue idle!");
	}
}
