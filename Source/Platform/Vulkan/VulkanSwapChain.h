#pragma once
#include "Renderer/SwapChain.h"
#include "VulkanSemaphore.h"
#include "VulkanTexture.h"
#include "VulkanUtils.h"

namespace Axiom {
    class VulkanSwapChain : public SwapChain {
      public:
        struct CreateInfo {
            Vk::Device logicDevice;
            Vk::SurfaceKHR surface;
            Vk::Queue presentQueue;
            SwapChainSupportDetails swapChainDetails;
            QueueFamilyIndices queueFamilyIndices;
            Vk::Extent2D windowSize;
        };

        VulkanSwapChain(const CreateInfo& createInfo);
        ~VulkanSwapChain() override;
        uint32_t acquireNextImage(Semaphore* imageAvailableSemaphore) override;
        Texture* getImageTexture(uint32_t index) override;
        bool present(uint32_t imageIndex, Semaphore* waitSemaphore) override;
        uint32_t getImageCount() const override;
        uint32_t getWidth() const override;
        uint32_t getHeight() const override;

      private:
        Vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<Vk::SurfaceFormatKHR>& availableFormats);
        Vk::PresentModeKHR chooseSwapPresentMode(const std::vector<Vk::PresentModeKHR>& availablePresentModes);
        Vk::Extent2D chooseSwapExtent(const Vk::SurfaceCapabilitiesKHR& capabilities, Vk::Extent2D windowSize) const;

      private:
        Vk::Device device = nullptr;
        Vk::SwapchainKHR swapChain = nullptr;
        Vk::Queue presentQueue = nullptr;
        std::vector<std::unique_ptr<VulkanTexture>> swapChainTextures;
        Vk::Extent2D swapChainExtent = {0, 0};
        Vk::Format swapChainImageFormat = Vk::Format::eUndefined;
    };
} // namespace Axiom
