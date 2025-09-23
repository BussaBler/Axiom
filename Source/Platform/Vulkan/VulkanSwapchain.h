#pragma once
#include "Renderer/Swapchain.h"
#include "VulkanContext.h"
#include "VulkanImage.h"
#include "Math/AxMath.h"
#include <vulkan/vulkan.h>

namespace Axiom {
	class VulkanDevice;
	class VulkanQueue;

	class VulkanSwapchain : public Swapchain {
	public:
		VulkanSwapchain(const SwapchainCreateInfo& swapchainCreateInfo, VulkanDevice& vkDevice, VulkanQueue& presentQueue);
		~VulkanSwapchain() override;

		void build();

		void rebuild(const SwapchainCreateInfo& swapchainCreateInfo) override;
		void present(Context& context) override;
		void prepare(Context& context) override;

		VkSwapchainKHR getHandle() const { return swapchain; }
		VkFormat getImageFormat() const { return swapChainImageFormat; }
		uint32_t getImageCount() const { return static_cast<uint32_t>(frames.size()); }
		VulkanImage& getImage(uint32_t index) { return *frameImages[index]; }
		VulkanImage& getDepthImage() { return *depthImage; }
		uint32_t getCurrentImageIndex() const { return currentImageIndex; }
		bool getIsRecreating() const { return isRecreating; }
		Math::uVec2 getExtent() const { return { swapchainCreateInfo.width, swapchainCreateInfo.height }; }

	private:
		VulkanDevice& device;
		VulkanQueue& presentQueue;
		VkSwapchainKHR swapchain;
		VkSurfaceKHR surface;
		VkFormat swapChainImageFormat;
		std::vector<VkImage> frames;
		std::vector<std::unique_ptr<VulkanImage>> frameImages;
		uint32_t currentImageIndex = 0;
		std::unique_ptr<VulkanImage> depthImage;
		bool isRecreating = false;
	};
}

