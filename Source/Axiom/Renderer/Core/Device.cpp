#include "Device.h"
#include "Platform/Vulkan/VulkanDevice.h"

namespace Axiom {
	std::unique_ptr<Device> Device::create(Window* window) {
		return std::make_unique<VulkanDevice>(window);
	}
}
