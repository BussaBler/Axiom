#pragma once
#include "Core/Assert.h"
#include "Renderer/Device.h"
#include "VulkanAdapter.h"
#include "VulkanQueue.h"

namespace Axiom {
    class VulkanDevice : public Device {
    public:
		VulkanDevice(VulkanAdapter& vkAdapter) : device(VK_NULL_HANDLE), adapter(vkAdapter) {}
		~VulkanDevice() override;

		void init(const DeviceCreateInfo& deviceCreateInfo) override;
		std::unique_ptr<Context> createContext() override;
		std::unique_ptr<Swapchain> createSwapchain(SwapchainCreateInfo& swapchainCreateInfo) override;
		std::unique_ptr<RenderPassCache> createRenderPassCache(Swapchain& swapchain) override;

		VkDevice getHandle() const { return device; }
		VulkanAdapter& getAdapter() const { return adapter; }
		VulkanQueue* getQueue(VkQueueFlags flags) {
			auto it = queues.find(flags);
			if (it != queues.end()) {
				return it->second.get();
			}
			AX_CORE_LOG_ERROR("Requested Vulkan Queue not found!");
			return nullptr;
		}
	private:
		std::unique_ptr<VulkanQueue> createQueue(VkQueueFlags flags);

    private:
		VkDevice device;
		VulkanAdapter& adapter;
		std::vector<QueueFamily> queueFamilies;
		std::map<VkQueueFlags, std::unique_ptr<VulkanQueue>> queues;
    };
}

