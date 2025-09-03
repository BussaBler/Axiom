#pragma once
#include "Renderer/Core/Queue.h"
#include "VulkanDevice.h"

namespace Axiom {
	class VulkanQueue : public Queue {
	public:
		VulkanQueue(VulkanDevice& vkDevice, uint32_t queueIndex);
		~VulkanQueue() override = default;
	};
}

