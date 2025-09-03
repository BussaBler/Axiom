#include "axpch.h"
#include "VulkanQueue.h"

namespace Axiom {
	VulkanQueue::VulkanQueue(VulkanDevice& vkDevice, uint32_t index) {
		handle.emplace<VkQueue>(VK_NULL_HANDLE);
		vkGetDeviceQueue(vkDevice.getHandle<VkDevice>(), index, 0, std::any_cast<VkQueue>(&handle));
	}
}
