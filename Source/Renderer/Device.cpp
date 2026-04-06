#include "Device.h"
#include "Platform/Vulkan/VulkanDevice.h"

namespace Axiom {
    std::unique_ptr<Device> Device::create(const CreateInfo &createInfo) {
        return std::make_unique<VulkanDevice>(createInfo);
    }
} // namespace Axiom