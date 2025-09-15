#pragma once
#include "Renderer/ResourceView.h"

namespace Axiom {
	class VulkanDevice;

	class VulkanView : public ResourceView {
	public:
		VulkanView(VulkanDevice& vkDevice) : device(vkDevice), id(-1) {}

		uint32_t generateId() override;

	protected:
		VulkanDevice& device;
		uint32_t id;

		static uint32_t currentId;
		static std::mutex idMutex;
	};
}

