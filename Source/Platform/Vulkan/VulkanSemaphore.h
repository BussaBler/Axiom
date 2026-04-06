#pragma once
#include "Core/Assert.h"
#include "Renderer/Semaphore.h"
#include "VulkanInclude.h"

namespace Axiom {
    class VulkanSemaphore : public Semaphore {
      public:
        VulkanSemaphore(Vk::Device logicDevice);
        ~VulkanSemaphore() override;

        inline Vk::Semaphore getHandle() const {
            return semaphore;
        }

      private:
        Vk::Device device = nullptr;
        Vk::Semaphore semaphore = nullptr;
    };
} // namespace Axiom
