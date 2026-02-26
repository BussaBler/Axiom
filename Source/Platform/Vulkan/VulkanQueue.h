#pragma once
#include "Core/Assert.h"
#include "VulkanInclude.h"

namespace Axiom {
	class VulkanDevice;

	class VulkanQueue {
	public:
		VulkanQueue(VulkanDevice& device, uint32_t familyIndex, uint32_t queueIndex);
		~VulkanQueue();

		void wait() const;

		Vk::Queue getHandle() const { return queue; }
		Vk::CommandPool getCommandPool() const { return commandPool; }
		uint32_t getFamilyIndex() const { return familyIndex; }
		uint32_t getQueueIndex() const { return queueIndex; }

	private:
		VulkanDevice& device;
		uint32_t familyIndex;
		uint32_t queueIndex;
		Vk::Queue queue;
		Vk::CommandPool commandPool;
	};
}

