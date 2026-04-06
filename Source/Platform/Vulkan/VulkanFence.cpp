#include "VulkanFence.h"

namespace Axiom {
    VulkanFence::VulkanFence(Vk::Device device, bool signaled) : device(device) {
        Vk::FenceCreateInfo fenceCreateInfo(signaled ? Vk::FenceCreateFlagBits::eSignaled : Vk::FenceCreateFlags{});
        Vk::ResultValue<Vk::Fence> fenceResult = device.createFence(fenceCreateInfo);

        AX_CORE_ASSERT(fenceResult.result == Vk::Result::eSuccess, "Failed to create Vulkan fence: {}", Vk::to_string(fenceResult.result));
        fence = fenceResult.value;
    }

    VulkanFence::~VulkanFence() {
        if (fence) {
            device.destroyFence(fence);
        }
    }

    void VulkanFence::wait() {
        Vk::Result result = device.waitForFences(fence, Vk::True, (std::numeric_limits<uint32_t>::max)());
        AX_CORE_ASSERT(result == Vk::Result::eSuccess, "Failed to wait for Vulkan fence: {}", Vk::to_string(result));
    }

    void VulkanFence::reset() {
        Vk::Result result = device.resetFences(fence);
        AX_CORE_ASSERT(result == Vk::Result::eSuccess, "Failed to reset Vulkan fence: {}", Vk::to_string(result));
    }
} // namespace Axiom
