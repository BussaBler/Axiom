#include "axpch.h"
#include "VulkanView.h"
#include "VulkanDevice.h"

namespace Axiom {
	uint32_t VulkanView::currentId = 0;
	std::mutex VulkanView::idMutex;

	uint32_t VulkanView::generateId() {
		std::lock_guard<std::mutex> lock(idMutex);
		return ++currentId;
	}
}
