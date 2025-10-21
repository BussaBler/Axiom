#pragma once
#include "Core/Assert.h"
#include <vulkan/vulkan.h>

namespace Axiom {
	class VulkanDevice;

	class VulkanQueue {
	public:
		VulkanQueue(VulkanDevice& device, uint32_t familyIndex, uint32_t queueIndex);
		~VulkanQueue();

		void wait() const;

		VkQueue getHandle() const { return queue; }
		VkCommandPool getCommandPool() const { return commandPool; }
		uint32_t getFamilyIndex() const { return familyIndex; }
		uint32_t getQueueIndex() const { return queueIndex; }

	private:
		VulkanDevice& device;
		uint32_t familyIndex;
		uint32_t queueIndex;
		VkQueue queue;
		VkCommandPool commandPool;
	};
}

