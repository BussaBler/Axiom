#pragma once
#include "VulkanDevice.h"
#include "Axiom/Renderer/Core/Fence.h"

namespace Axiom {
	class VulkanFence : public Fence {
	public:
		VulkanFence(VulkanDevice& device, bool signaled = false);
		~VulkanFence();

		void wait(uint64_t timeout) override;
		void reset() override;

	private:
		VulkanDevice& device;
	};
}

