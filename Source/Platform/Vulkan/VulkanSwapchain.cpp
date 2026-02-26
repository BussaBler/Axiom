#include "axpch.h"
#include "VulkanSwapchain.h"
#include "VulkanDevice.h"
#include "VulkanInstance.h"
#include "Math/AxMath.h"

namespace Axiom {
	VulkanSwapchain::VulkanSwapchain(const SwapchainCreateInfo& swapchainCreateInfo, VulkanDevice& vkDevice, VulkanQueue& presentQueue)
		: Swapchain(swapchainCreateInfo), device(vkDevice), presentQueue(presentQueue), swapchain(nullptr), surface(nullptr),
			swapChainImageFormat(Vk::Format::eUndefined) {

	}

	VulkanSwapchain::~VulkanSwapchain() {
		AX_CORE_LOG_INFO("Destroying Vulkan Swapchain...");
		if (swapchain) {
			device.getHandle().destroySwapchainKHR(swapchain);
			swapchain = nullptr;
		}
		if (surface) {
			device.getAdapter().getInstance().getHandle().destroySurfaceKHR(surface);
			surface = nullptr;
		}

	}

	void VulkanSwapchain::build() {
		if (!swapchain) {
			AX_CORE_LOG_INFO("Building Vulkan Swapchain...");
		}
		Vk::SurfaceKHR newSurface = device.getAdapter().getInstance().createSurface(swapchainCreateInfo.windowHandle);
		auto surfaceFormats = device.getAdapter().getSurfaceFormats(newSurface);
		auto presentModes = device.getAdapter().getPresentModes(newSurface);
		uint32_t frameCount = swapchainCreateInfo.desiredFrameCount;
		Vk::SurfaceCapabilitiesKHR surfaceCapabilities = device.getAdapter().getSurfaceCapabilities(newSurface);
		Vk::ImageUsageFlags supportedUsageFlags = surfaceCapabilities.supportedUsageFlags;

		Vk::SurfaceFormatKHR swapChainSurfaceFormat = surfaceFormats[0];
		for (const auto& format : surfaceFormats) {
			if (format.format == Vk::Format::eB8G8R8A8Unorm && format.colorSpace == Vk::ColorSpaceKHR::eSrgbNonlinear) {
				swapChainSurfaceFormat = format;
				break;
			}
		}
		swapChainImageFormat = swapChainSurfaceFormat.format;
		frameCount = Math::axClamp(frameCount, surfaceCapabilities.minImageCount, surfaceCapabilities.maxImageCount);

		// TODO: add a config for this
		Vk::PresentModeKHR swapChainPresentMode = Vk::PresentModeKHR::eFifo;
		for (const auto& presentMode : presentModes) {
			if (presentMode == Vk::PresentModeKHR::eMailbox) {
				swapChainPresentMode = presentMode;
				break;
			}
			else if (presentMode == Vk::PresentModeKHR::eImmediate) {
				swapChainPresentMode = presentMode;
			}
		}

		Vk::Extent2D swapChainExtent(swapchainCreateInfo.width, swapchainCreateInfo.height);
		if (surfaceCapabilities.currentExtent.width != UINT32_MAX) {
			swapChainExtent = surfaceCapabilities.currentExtent;
		}
		else {
			swapChainExtent.width = Math::axClamp(swapChainExtent.width, surfaceCapabilities.minImageExtent.width, surfaceCapabilities.maxImageExtent.width);
			swapChainExtent.height = Math::axClamp(swapChainExtent.height, surfaceCapabilities.minImageExtent.height, surfaceCapabilities.maxImageExtent.height);
		}

		Vk::SwapchainCreateInfoKHR createInfo(
			{},
			newSurface,
			frameCount,
			swapChainSurfaceFormat.format,
			swapChainSurfaceFormat.colorSpace,
			swapChainExtent,
			1,
			Vk::ImageUsageFlagBits::eColorAttachment,
			Vk::SharingMode::eExclusive,
			0,
			surfaceCapabilities.currentTransform,
			Vk::CompositeAlphaFlagBitsKHR::eOpaque,
			swapChainPresentMode,
			Vk::True,
			swapchain ? swapchain : nullptr
		);
		Vk::ResultValue<Vk::SwapchainKHR> newSwapchainResult = device.getHandle().createSwapchainKHR(createInfo);

		if (newSwapchainResult.result != Vk::Result::eSuccess) {
			AX_CORE_LOG_ERROR("Failed to create swapchain! VkResult: {}", Vk::to_string(newSwapchainResult.result));
			return;
		}

		if (swapchain) {
			device.getHandle().destroySwapchainKHR(swapchain);
			device.getAdapter().getInstance().getHandle().destroySurfaceKHR(surface);
		}
		
		swapchain = newSwapchainResult.value;
		surface = newSurface;

		Vk::ResultValue<std::vector<Vk::Image>> framesResult = device.getHandle().getSwapchainImagesKHR(swapchain);
		AX_CORE_ASSERT(framesResult.result == Vk::Result::eSuccess, "Failed to get swapchain images!");
		frames = framesResult.value;

		ResourceViewCreateInfo resourceViewCreateInfo{};
		resourceViewCreateInfo.format = VulkanResource::getResourceFormat(swapChainSurfaceFormat.format);
		resourceViewCreateInfo.aspectMask = ResourceAspectMask::Color;

		frameImages.clear();
		frameImages.reserve(frames.size());
		for (uint32_t i = 0; i < frames.size(); i++) {
			frameImages.push_back(std::make_unique<VulkanImage>(device, frames[i], Vk::ImageLayout::ePresentSrcKHR));
			frameImages[i]->getView(resourceViewCreateInfo);
		}

		depthImage = std::make_unique<VulkanImage>(device, nullptr, Vk::ImageLayout::eUndefined);
		ResourceCreateInfo depthImageCreateInfo{};
		depthImageCreateInfo.width = swapchainCreateInfo.width;
		depthImageCreateInfo.height = swapchainCreateInfo.height;
		depthImageCreateInfo.format = VulkanResource::getResourceFormat(device.getAdapter().getDepthFormat());
		depthImageCreateInfo.usage = ResourceUsage::DepthStencil;
		depthImage->init(depthImageCreateInfo);

		ResourceViewCreateInfo depthResourceViewCreateInfo{};
		depthResourceViewCreateInfo.format = depthImageCreateInfo.format;
		depthResourceViewCreateInfo.aspectMask = ResourceAspectMask::Depth | ResourceAspectMask::Stencil;
		depthImage->getView(depthResourceViewCreateInfo);
	}

	void VulkanSwapchain::rebuild(const SwapchainCreateInfo& swapchainCreateInfo) {
		this->swapchainCreateInfo = swapchainCreateInfo;
		isRecreating = true;
		device.waitIdle();
		build();
		isRecreating = false;
	}

	void VulkanSwapchain::present(Context& context) {
		VulkanContext& vkContext = static_cast<VulkanContext&>(context);
		std::array<Vk::SwapchainKHR, 1> swapchains = { swapchain };
		std::array<Vk::Semaphore, 1> waitSemaphores = { vkContext.getCurrentRenderFinishedSemaphore() };

		Vk::PresentInfoKHR presentInfo(waitSemaphores, swapchains, currentImageIndex);


		Vk::Result result = presentQueue.getHandle().presentKHR(presentInfo);
		if (result == Vk::Result::eErrorOutOfDateKHR || result == Vk::Result::eSuboptimalKHR) {
			AX_CORE_LOG_WARN("Swapchain out of date during present, needs to be rebuilt");
			isRecreating = true;
			return;
		}
		vkContext.incrementFrameIndex();
	}

	void VulkanSwapchain::prepare(Context& context) {
		VulkanContext& vkContext = static_cast<VulkanContext&>(context);
		AX_CORE_ASSERT(vkContext.getFrameCount() <= frameImages.size(), "Not enough swapchain images for the context frame count!");

		VulkanContextFrame currentFrame = vkContext.getCurrentFrameResource();
		Vk::ResultValue<uint32_t> acquireNextImageResult = device.getHandle().acquireNextImageKHR(swapchain, UINT64_MAX, currentFrame.imageAvailableSemaphore, {});

		switch (acquireNextImageResult.result) {
			case Vk::Result::eErrorOutOfDateKHR:
				AX_CORE_LOG_WARN("Swapchain out of date during acquire, needs to be rebuilt");
				isRecreating = true;
				return;
				break;
			default:
				AX_CORE_ASSERT(acquireNextImageResult.result == Vk::Result::eSuccess || acquireNextImageResult.result == Vk::Result::eSuboptimalKHR, "Failed to acquire swapchain image!");
			break;
		}
		currentImageIndex = acquireNextImageResult.value;
	}
}
