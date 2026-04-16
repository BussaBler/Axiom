#pragma once
#include "Renderer/SwapChain.h"
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
            uint32_t maxFramesInFlight;
        };

        VulkanSwapChain(const CreateInfo& createInfo);
        ~VulkanSwapChain() override;
        bool acquireNextImage() override;
        Texture* getCurrentTexture() override;
        Format getTextureFormat() const override;
        bool present() override;
        uint32_t getWidth() const override;
        uint32_t getHeight() const override;

        Vk::Semaphore getImageAvailableSemaphore(uint32_t index) const { return imageAvailableSemaphores[index]; }
        Vk::Semaphore getRenderFinishedSemaphore() const { return renderFinishedSemaphores[currentImageIndex]; }
        void setCurrentFrameIndex(uint32_t index) { currentFrameIndex = index; }

      private:
        Vk::SurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<Vk::SurfaceFormatKHR>& availableFormats);
        Vk::PresentModeKHR chooseSwapPresentMode(const std::vector<Vk::PresentModeKHR>& availablePresentModes);
        Vk::Extent2D chooseSwapExtent(const Vk::SurfaceCapabilitiesKHR& capabilities, Vk::Extent2D windowSize) const;

      private:
        Vk::Device device = nullptr;
        Vk::SwapchainKHR swapChain = nullptr;
        Vk::Queue presentQueue = nullptr;
        uint32_t currentImageIndex = 0;
        uint32_t currentFrameIndex = 0;
        std::vector<std::unique_ptr<VulkanTexture>> swapChainTextures;
        std::vector<Vk::Semaphore> imageAvailableSemaphores;
        std::vector<Vk::Semaphore> renderFinishedSemaphores;
        Vk::Extent2D swapChainExtent = {0, 0};
        Vk::Format swapChainImageFormat = Vk::Format::eUndefined;
    };
} // namespace Axiom
