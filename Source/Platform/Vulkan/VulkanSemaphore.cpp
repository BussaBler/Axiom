#include "axpch.h"
#include "VulkanSemaphore.h"

namespace Axiom {
	VulkanSemaphore::VulkanSemaphore(VulkanDevice& vkDevice) : device(vkDevice) {
		VkSemaphoreCreateInfo semaphoreInfo = {};
		semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		handle.emplace<VkSemaphore>(VK_NULL_HANDLE);
		AX_CORE_ASSERT(vkCreateSemaphore(device.getHandle<VkDevice>(), &semaphoreInfo, nullptr, std::any_cast<VkSemaphore>(&handle)) == VK_SUCCESS, "Failed to create Vulkan semaphore");
	}

	VulkanSemaphore::~VulkanSemaphore() {
		vkDeviceWaitIdle(device.getHandle<VkDevice>());
		vkDestroySemaphore(device.getHandle<VkDevice>(), std::any_cast<VkSemaphore>(handle), nullptr);
	}
}
		
