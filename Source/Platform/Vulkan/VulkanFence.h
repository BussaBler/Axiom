#pragma once
#include "VulkanDevice.h"

namespace Axiom {
	class VulkanFence {
	public:
		VulkanFence(VulkanDevice& device, bool signaled = false);
		~VulkanFence();

		VkFence getHandle() const { return handle; }

		void wait(uint64_t timeout);
		void reset();

	private:
		VulkanDevice& device;
		VkFence handle = VK_NULL_HANDLE;
		bool signaled = false;
	};
}

