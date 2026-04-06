#include "VulkanSemaphore.h"

namespace Axiom {
    VulkanSemaphore::VulkanSemaphore(Vk::Device logicDevice) : device(logicDevice) {
        Vk::SemaphoreCreateInfo semaphoreCreateInfo{};
        Vk::ResultValue<Vk::Semaphore> semaphoreResult = device.createSemaphore(semaphoreCreateInfo);

        AX_CORE_ASSERT(semaphoreResult.result == Vk::Result::eSuccess, "Failed to create Vulkan semaphore: {}", Vk::to_string(semaphoreResult.result));
        semaphore = semaphoreResult.value;
    }

    VulkanSemaphore::~VulkanSemaphore() {
        if (semaphore) {
            device.destroySemaphore(semaphore);
        }
    }
} // namespace Axiom
