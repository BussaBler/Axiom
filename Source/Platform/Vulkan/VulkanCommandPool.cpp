#include "axpch.h"
#include "VulkanCommandPool.h"

namespace Axiom {
	VulkanCommandPool::VulkanCommandPool(VulkanDevice& vkDevice, uint32_t queueFamilyIndex) : device(vkDevice) {
		VkCommandPoolCreateInfo poolInfo{};
		poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		poolInfo.queueFamilyIndex = queueFamilyIndex;
		poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		handle.emplace<VkCommandPool>(VK_NULL_HANDLE);
		AX_CORE_ASSERT(vkCreateCommandPool(vkDevice.getHandle<VkDevice>(), &poolInfo, nullptr, reinterpret_cast<VkCommandPool*>(&handle)) == VK_SUCCESS, "Failed to create command pool!");
	}

	VulkanCommandPool::~VulkanCommandPool() {
		if (handle.has_value()) {
			vkDestroyCommandPool(device.getHandle<VkDevice>(), getHandle<VkCommandPool>(), nullptr);
		}
	}
}

