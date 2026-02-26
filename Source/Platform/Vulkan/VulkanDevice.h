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
		std::unique_ptr<Shader> createShader(RenderPassToken& token) override;
		std::unique_ptr<Resource> createResource(ResourceCreateInfo& resourceCreateInfo) override;
		std::unique_ptr<Texture> createTexture(TextureCreateInfo& textureCreateInfo) override;

		void waitIdle() const;

		Vk::Device getHandle() const { return device; }
		VulkanAdapter& getAdapter() const { return adapter; }
		VulkanQueue* getQueue(Vk::QueueFlags flags) {
			auto it = queues.find(flags);
			if (it != queues.end()) {
				return it->second;
			}
			AX_CORE_LOG_ERROR("Requested Vulkan Queue not found!");
			return nullptr;
		}
	private:
		std::unique_ptr<VulkanQueue> createQueue(Vk::QueueFlags flags);

    private:
		Vk::Device device;
		VulkanAdapter& adapter;
		std::vector<QueueFamily> queueFamilies;
		std::unique_ptr<VulkanQueue> mainQueue;
		std::map<Vk::QueueFlags, VulkanQueue*> queues;
    };
}

