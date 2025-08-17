#include "axpch.h"
#include "VulkanFence.h"

namespace Axiom {
	VulkanFence::VulkanFence(VulkanDevice& device, bool signaled)
		: device(device), signaled(signaled) {
		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;
		AX_CORE_ASSERT(vkCreateFence(device.getHandle(), &fenceInfo, nullptr, &handle) == VK_SUCCESS, "Failed to create Vulkan fence");
	}

	VulkanFence::~VulkanFence() {
		if (handle != VK_NULL_HANDLE) {
			vkDestroyFence(device.getHandle(), handle, nullptr);
			handle = VK_NULL_HANDLE;
		}
	}

	void VulkanFence::wait(uint64_t timeout) {
		if (signaled) {
			return;
		}
		auto result = vkWaitForFences(device.getHandle(), 1, &handle, VK_TRUE, timeout);
		switch (result) {
			case VK_SUCCESS:
				signaled = true;
				break;
			case VK_TIMEOUT:
				AX_CORE_LOG_WARN("Vulkan fence wait timed out after {} milliseconds", timeout);
				break;
			case VK_ERROR_OUT_OF_HOST_MEMORY:
				AX_CORE_LOG_ERROR("Vulkan fence wait failed: Out of host memory");
				break;
			case VK_ERROR_OUT_OF_DEVICE_MEMORY:
				AX_CORE_LOG_ERROR("Vulkan fence wait failed: Out of device memory");
				break;
			case VK_ERROR_DEVICE_LOST:
				AX_CORE_LOG_ERROR("Vulkan fence wait failed: Device lost");
				break;
			default:
				AX_CORE_LOG_ERROR("Vulkan fence wait failed with unknown error code: {}", static_cast<int>(result));
				break;
		}
	}

	void VulkanFence::reset() {
		AX_CORE_ASSERT(vkResetFences(device.getHandle(), 1, &handle) == VK_SUCCESS, "Failed to reset Vulkan fence");
		signaled = false;
	}
}
