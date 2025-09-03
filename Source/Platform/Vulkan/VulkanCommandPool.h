#pragma once
#include "Renderer/Core/CommandPool.h"
#include "VulkanDevice.h"

namespace Axiom {
	class VulkanCommandPool : public CommandPool {
		public:
		VulkanCommandPool(VulkanDevice& vkDevice, uint32_t queueFamilyIndex);
		~VulkanCommandPool() override;

	private:
		VulkanDevice& device;
	};
}

