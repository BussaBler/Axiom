#pragma once
#include "Renderer/Core/Semaphore.h"
#include "VulkanDevice.h"

namespace Axiom {
	class VulkanSemaphore : public Semaphore {
	public:
		VulkanSemaphore(VulkanDevice& vkDevice);
		~VulkanSemaphore() override;

	private:
		VulkanDevice& device;
	};
}

