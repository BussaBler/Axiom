#include "axpch.h"
#include "VulkanFence.h"

namespace Axiom {
	VulkanFence::VulkanFence(VulkanDevice& device, bool signaled)
		: Fence(signaled), device(device) {
		VkFenceCreateInfo fenceInfo = {};
		fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceInfo.flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0;
		handle.emplace<VkFence>(VK_NULL_HANDLE);
		AX_CORE_ASSERT(vkCreateFence(device.getHandle<VkDevice>(), &fenceInfo, nullptr, std::any_cast<VkFence>(&handle)) == VK_SUCCESS, "Failed to create Vulkan fence");
	}

	VulkanFence::~VulkanFence() {
		vkDeviceWaitIdle(device.getHandle<VkDevice>());
		if (handle.has_value()) {
			vkDestroyFence(device.getHandle<VkDevice>(), getHandle<VkFence>(), nullptr);
			handle.reset();
		}
	}

	void VulkanFence::wait(uint64_t timeout) {
		if (signaled) {
			return;
		}
		auto result = vkWaitForFences(device.getHandle<VkDevice>(), 1, getHandlePtr<VkFence>(), VK_TRUE, timeout);
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
		AX_CORE_ASSERT(vkResetFences(device.getHandle<VkDevice>(), 1, getHandlePtr<VkFence>()) == VK_SUCCESS, "Failed to reset Vulkan fence");
		signaled = false;
	}
}
