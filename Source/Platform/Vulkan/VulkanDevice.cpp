#include "axpch.h"
#include "VulkanDevice.h"
#include "VulkanContext.h"
#include "VulkanSwapchain.h"
#include "VulkanRenderPass.h"
#include "VulkanRenderPassCache.h"
#include "VulkanBuffer.h"
#include "VulkanTexture.h"
// temp
#include "VulkanMaterialShader.h"

namespace Axiom {
	VulkanDevice::~VulkanDevice() {
		AX_CORE_LOG_INFO("Destroying Vulkan Logical Device...");
		mainQueue.reset();
		device.destroy();
	}

	void VulkanDevice::init(const DeviceCreateInfo& deviceCreateInfo) {
		AX_CORE_LOG_INFO("Initializing Vulkan Logical Device...");
		queueFamilies = adapter.findQueueFamilies();

		std::vector<Vk::DeviceQueueCreateInfo> queueCreateInfos(queueFamilies.size());
		std::array<float, 1> queuePriority = { 1.0f };
		for (uint32_t i = 0; i < queueFamilies.size(); i++) {
			queueCreateInfos[i] = Vk::DeviceQueueCreateInfo({}, queueFamilies[i].index, queuePriority);
		}

		std::array<const char*, 0> enabledLayerNames = {};
		std::array<const char*, 1> enabledExtensionsNames = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

		Vk::PhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.setSamplerAnisotropy(Vk::True);

		Vk::DeviceCreateInfo createInfo({}, queueCreateInfos, enabledLayerNames, enabledExtensionsNames, &deviceFeatures);
		Vk::ResultValue<Vk::Device> deviceResult = adapter.getHandle().createDevice(createInfo);

		AX_CORE_ASSERT(deviceResult.result == Vk::Result::eSuccess, "Failed to create logical device!");
		device = deviceResult.value;
		VULKAN_HPP_DEFAULT_DISPATCHER.init(device);
	
		mainQueue = createQueue(Vk::QueueFlagBits::eGraphics | Vk::QueueFlagBits::eTransfer | Vk::QueueFlagBits::eCompute);
		if (mainQueue) {
			queues[Vk::QueueFlagBits::eGraphics | Vk::QueueFlagBits::eTransfer | Vk::QueueFlagBits::eCompute] = mainQueue.get();
			queues[Vk::QueueFlagBits::eGraphics] = mainQueue.get();
			queues[Vk::QueueFlagBits::eTransfer] = mainQueue.get();
			queues[Vk::QueueFlagBits::eCompute] = mainQueue.get();
		} else {
			AX_CORE_LOG_ERROR("Failed to create Main Vulkan Queue!");
		}
	}

	std::unique_ptr<Context> VulkanDevice::createContext() {
		std::unique_ptr<VulkanContext> context = std::make_unique<VulkanContext>(*this, *getQueue(Vk::QueueFlagBits::eGraphics | Vk::QueueFlagBits::eTransfer | Vk::QueueFlagBits::eCompute));
		context->init(3);
		return std::move(context);
	}

	std::unique_ptr<Swapchain> VulkanDevice::createSwapchain(SwapchainCreateInfo& swapchainCreateInfo) {
		std::unique_ptr<VulkanSwapchain> swapchain = std::make_unique<VulkanSwapchain>(swapchainCreateInfo, *this, *getQueue(Vk::QueueFlagBits::eGraphics | Vk::QueueFlagBits::eTransfer | Vk::QueueFlagBits::eCompute));
		swapchain->build();
		return std::move(swapchain);
	}

	std::unique_ptr<RenderPassCache> VulkanDevice::createRenderPassCache(Swapchain& swapchain) {
		return std::make_unique<VulkanRenderPassCache>(*this, static_cast<VulkanSwapchain&>(swapchain));
	}

	std::unique_ptr<Shader> VulkanDevice::createShader(RenderPassToken& token) {
		auto renderPass = static_cast<VulkanRenderPass*>(token.get());
		auto shader = std::make_unique<VulkanMaterialShader>(*this);
		shader->init(renderPass->getHandle());
		return std::move(shader);
	}

	std::unique_ptr<Resource> VulkanDevice::createResource(ResourceCreateInfo& resourceCreateInfo) {
		switch (resourceCreateInfo.type) {
			case ResourceType::Buffer: {
				auto buffer = std::make_unique<VulkanBuffer>(*this);
				buffer->init(resourceCreateInfo);
				return std::move(buffer);
				break;
			}
			default:
				AX_CORE_LOG_ERROR("Unsupported resource type!");
				return nullptr;
				break;
		}
	}

	std::unique_ptr<Texture> VulkanDevice::createTexture(TextureCreateInfo& textureCreateInfo) {
		std::unique_ptr<VulkanTexture> texture = std::make_unique<VulkanTexture>(*this);
		texture->init(textureCreateInfo);
		return std::move(texture);
	}

	void VulkanDevice::waitIdle() const {
		AX_CORE_ASSERT(device.waitIdle() == Vk::Result::eSuccess, "Failed to wait for device idle!");
	}

	std::unique_ptr<VulkanQueue> VulkanDevice::createQueue(Vk::QueueFlags flags) {
		uint32_t index = -1;
		for (uint32_t i = 0; i < queueFamilies.size(); i++) {
			if (queueFamilies[i].flags & flags) {
				if (queueFamilies[i].availableIndex < queueFamilies[i].maxQueues) {
					index = i;
					break;
				}
			}
		}

		if (index == -1) {
			AX_CORE_LOG_ERROR("Failed to find a suitable queue family!");
			return nullptr;
		}

		return std::make_unique<VulkanQueue>(*this, index, queueFamilies[index].availableIndex++);
	}
}
