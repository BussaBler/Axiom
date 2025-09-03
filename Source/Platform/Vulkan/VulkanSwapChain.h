#pragma once
#include "VulkanDevice.h" 
#include "VulkanImage.h"
#include "VulkanImageView.h"
#include "Renderer/Core/SwapChain.h"

namespace Axiom {
	class VulkanSwapChain : public SwapChain {
	public:
		static constexpr int MAX_FRAMES_IN_FLIGHT = 2;

		VulkanSwapChain(VulkanDevice& device, Math::uVec2 extent);
		~VulkanSwapChain();

		void recreate(Math::uVec2 extent) override;
		void acquireNextImageIndex(uint32_t& imageIndex, const Semaphore& semaphore, Fence* fence, Math::uVec2 extent) override;
		void presentImage(const Queue& graphicsQueue, const Queue& presentQueue, const Semaphore& renderSemaphore, uint32_t imageIndex, uint32_t& currentFrame, Math::uVec2 extent) override;

		VkSurfaceFormatKHR getImageFormat() const { return swapChainImageFormat; }
		uint32_t getImageCount() const { return swapChainImages.size(); }
		const std::vector<std::unique_ptr<VulkanImageView>>& getImageViews() const { return swapChainImageViews; }
		VulkanImage* getDepthImage() { return depthImage.get(); }
		VulkanImageView& getDepthImageView() { return *depthImageView; }

	private:
		void createSwapChain(VkExtent2D extent);
		void destroySwapChain();

	private:
		VulkanDevice& device;
		VkSurfaceFormatKHR swapChainImageFormat;
		VkExtent2D swapChainExtent;

		std::unique_ptr<VulkanImage> depthImage;
		std::unique_ptr<VulkanImageView> depthImageView;
		std::vector<VkImage> swapChainImages;
		std::vector<std::unique_ptr<VulkanImageView>> swapChainImageViews;
	};
}

