#include "VulkanSwapChain.h"
#include "Math/AxMath.h"

namespace Axiom {
    VulkanSwapChain::VulkanSwapChain(const CreateInfo& createInfo) : device(createInfo.logicDevice), presentQueue(createInfo.presentQueue) {
        Vk::SurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(createInfo.swapChainDetails.formats);
        Vk::PresentModeKHR presentMode = chooseSwapPresentMode(createInfo.swapChainDetails.presentModes);
        Vk::Extent2D extent = chooseSwapExtent(createInfo.swapChainDetails.capabilities, createInfo.windowSize);

        uint32_t imageCount = createInfo.swapChainDetails.capabilities.minImageCount + 1;
        if (createInfo.swapChainDetails.capabilities.maxImageCount > 0 && imageCount > createInfo.swapChainDetails.capabilities.maxImageCount) {
            imageCount = createInfo.swapChainDetails.capabilities.maxImageCount;
        }

        Vk::SwapchainCreateInfoKHR swapChainCreateInfo({}, createInfo.surface, imageCount, surfaceFormat.format, surfaceFormat.colorSpace, extent, 1,
                                                       Vk::ImageUsageFlagBits::eColorAttachment);

        std::array<uint32_t, 2> queueFamilyIndices = {createInfo.queueFamilyIndices.graphicsFamily.value(),
                                                      createInfo.queueFamilyIndices.presentFamily.value()};
        if (createInfo.queueFamilyIndices.graphicsFamily != createInfo.queueFamilyIndices.presentFamily) {
            swapChainCreateInfo.setImageSharingMode(Vk::SharingMode::eConcurrent);
            swapChainCreateInfo.setQueueFamilyIndices(queueFamilyIndices);
        } else {
            swapChainCreateInfo.setImageSharingMode(Vk::SharingMode::eExclusive);
        }

        swapChainCreateInfo.setPreTransform(createInfo.swapChainDetails.capabilities.currentTransform);
        swapChainCreateInfo.setCompositeAlpha(Vk::CompositeAlphaFlagBitsKHR::eOpaque);
        swapChainCreateInfo.setPresentMode(presentMode);
        swapChainCreateInfo.setClipped(Vk::True);
        Vk::ResultValue<Vk::SwapchainKHR> swapChainResult = device.createSwapchainKHR(swapChainCreateInfo);

        AX_CORE_ASSERT(swapChainResult.result == Vk::Result::eSuccess, "Failed to create swap chain!");
        swapChain = swapChainResult.value;

        Vk::ResultValue<std::vector<Vk::Image>> swapChainImagesResult = device.getSwapchainImagesKHR(swapChain);
        AX_CORE_ASSERT(swapChainImagesResult.result == Vk::Result::eSuccess, "Failed to get swap chain images!");
        swapChainExtent = extent;
        swapChainImageFormat = surfaceFormat.format;

        swapChainTextures.reserve(swapChainImagesResult.value.size());
        for (size_t i = 0; i < swapChainImagesResult.value.size(); i++) {
            swapChainTextures.push_back(std::make_unique<VulkanTexture>(device, swapChainImagesResult.value[i]));
            swapChainTextures[i]->createImageView(swapChainImageFormat, Vk::ImageAspectFlagBits::eColor);
        }

        imageAvailableSemaphores.resize(createInfo.maxFramesInFlight);
        for (size_t i = 0; i < createInfo.maxFramesInFlight; i++) {
            Vk::SemaphoreCreateInfo semaphoreCreateInfo;
            Vk::ResultValue<Vk::Semaphore> imageAvailableSemaphoreResult = device.createSemaphore(semaphoreCreateInfo);
            AX_CORE_ASSERT(imageAvailableSemaphoreResult.result == Vk::Result::eSuccess, "Failed to create image available semaphore!");
            imageAvailableSemaphores[i] = imageAvailableSemaphoreResult.value;
        }

        renderFinishedSemaphores.resize(swapChainTextures.size());
        for (size_t i = 0; i < renderFinishedSemaphores.size(); i++) {
            Vk::SemaphoreCreateInfo semaphoreCreateInfo;
            Vk::ResultValue<Vk::Semaphore> renderFinishedSemaphoreResult = device.createSemaphore(semaphoreCreateInfo);
            AX_CORE_ASSERT(renderFinishedSemaphoreResult.result == Vk::Result::eSuccess, "Failed to create render finished semaphore!");
            renderFinishedSemaphores[i] = renderFinishedSemaphoreResult.value;
        }
    }

    VulkanSwapChain::~VulkanSwapChain() {
        device.destroySwapchainKHR(swapChain);
    }

    bool VulkanSwapChain::acquireNextImage() {
        Vk::Semaphore vkSignal = imageAvailableSemaphores[currentFrameIndex];
        Vk::ResultValue<uint32_t> acquireResult = device.acquireNextImageKHR(swapChain, (std::numeric_limits<uint64_t>::max)(), vkSignal, nullptr);

        if (acquireResult.result == Vk::Result::eErrorOutOfDateKHR) {
            return false;
        } else if (acquireResult.result != Vk::Result::eSuccess && acquireResult.result != Vk::Result::eSuboptimalKHR) {
            AX_CORE_ASSERT(false, "Failed to acquire swap chain image!");
            return false;
        }

        currentImageIndex = acquireResult.value;
        return true;
    }

    Texture* VulkanSwapChain::getCurrentTexture() {
        return swapChainTextures[currentImageIndex].get();
    }

    Format VulkanSwapChain::getTextureFormat() const {
        return vkToAxFormat(swapChainImageFormat);
    }

    bool VulkanSwapChain::present() {
        Vk::Semaphore vkWait = renderFinishedSemaphores[currentImageIndex];
        Vk::PresentInfoKHR presentInfo(vkWait, swapChain, currentImageIndex);

        Vk::Result presentResult = presentQueue.presentKHR(presentInfo);

        if (presentResult == Vk::Result::eErrorOutOfDateKHR || presentResult == Vk::Result::eSuboptimalKHR) {
            return false;
        } else if (presentResult != Vk::Result::eSuccess) {
            AX_CORE_ASSERT(false, "Failed to present swap chain image!");
            return false;
        }

        return true;
    }

    uint32_t VulkanSwapChain::getWidth() const {
        return swapChainExtent.width;
    }

    uint32_t VulkanSwapChain::getHeight() const {
        return swapChainExtent.height;
    }

    Vk::SurfaceFormatKHR VulkanSwapChain::chooseSwapSurfaceFormat(const std::vector<Vk::SurfaceFormatKHR>& availableFormats) {
        for (const auto& availableFormat : availableFormats) {
            if (availableFormat.format == Vk::Format::eB8G8R8A8Srgb && availableFormat.colorSpace == Vk::ColorSpaceKHR::eSrgbNonlinear) {
                return availableFormat;
            }
        }

        return availableFormats[0];
    }

    Vk::PresentModeKHR VulkanSwapChain::chooseSwapPresentMode(const std::vector<Vk::PresentModeKHR>& availablePresentModes) {
        for (const auto& availablePresentMode : availablePresentModes) {
            if (availablePresentMode == Vk::PresentModeKHR::eMailbox) {
                return availablePresentMode;
            }
        }

        return Vk::PresentModeKHR::eFifo;
    }

    Vk::Extent2D VulkanSwapChain::chooseSwapExtent(const Vk::SurfaceCapabilitiesKHR& capabilities, Vk::Extent2D windowSize) const {
        if (capabilities.currentExtent.width != (std::numeric_limits<uint32_t>::max)()) {
            return capabilities.currentExtent;
        } else {
            Vk::Extent2D actualExtent = windowSize;
            actualExtent.width = Math::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
            actualExtent.height = Math::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
            return actualExtent;
        }
    }
} // namespace Axiom
