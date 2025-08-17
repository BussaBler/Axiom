#pragma once
#include "VulkanDevice.h" 
#include "VulkanImage.h"

namespace Axiom {
	class VulkanSwapChain {
	public:
		static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

		VulkanSwapChain(VulkanDevice& device, VkExtent2D extent);
		~VulkanSwapChain();

		void recreate(VkExtent2D extent);

		VulkanSwapChain(const VulkanSwapChain&) = delete;
		void operator=(const VulkanSwapChain&) = delete;

		VkSurfaceFormatKHR getImageFormat() const { return swapChainImageFormat; }
		uint32_t getImageCount() const { return swapChainImages.size(); }
		const std::vector<VkImageView> getImageViews() const { return swapChainImageViews; }
		VulkanImage getDepthImage() const { return depthImage; }

		void acquireNextImageIndex(uint32_t& imageIndex, VkSemaphore semaphore, VkFence fence, VkExtent2D extent);
		void presentImage(VkQueue graphicsQueue, VkQueue presentQueue, VkSemaphore renderSemaphore, uint32_t imageIndex, uint32_t& currentFrame, VkExtent2D extent);

	private:
		void createSwapChain(VkExtent2D extent);
		void destroySwapChain();

	private:
		VulkanDevice& device;
		VkSwapchainKHR handle;
		VkSurfaceFormatKHR swapChainImageFormat;
		VkExtent2D swapChainExtent;

		VulkanImage depthImage;
		std::vector<VkImage> swapChainImages;
		std::vector<VkImageView> swapChainImageViews;
	};
}

