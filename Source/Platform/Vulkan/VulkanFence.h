#pragma once
#include "Renderer/Fence.h"
#include "Core/Assert.h"
#include "VulkanInclude.h"

namespace Axiom {
	class VulkanFence : public Fence{
	public:
		VulkanFence(Vk::Device logicDevice, bool signaled = false);
		~VulkanFence() override;

		void wait() override;
		void reset() override;

		inline Vk::Fence getHandle() const { return fence; }

	private:
		Vk::Device device = nullptr;
		Vk::Fence fence = nullptr;
	};
}

