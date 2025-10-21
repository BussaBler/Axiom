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
		vkDestroyDevice(device, nullptr);
	}

	void VulkanDevice::init(const DeviceCreateInfo& deviceCreateInfo) {
		AX_CORE_LOG_INFO("Initializing Vulkan Logical Device...");
		queueFamilies = adapter.findQueueFamilies();

		std::vector<VkDeviceQueueCreateInfo> queueCreateInfos(queueFamilies.size());
		float queuePriority = 1.0f;
		for (uint32_t i = 0; i < queueFamilies.size(); i++) {
			queueCreateInfos[i].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
			queueCreateInfos[i].queueFamilyIndex = queueFamilies[i].index;
			queueCreateInfos[i].queueCount = 1;
			queueCreateInfos[i].pQueuePriorities = &queuePriority;
		}

		VkPhysicalDeviceFeatures deviceFeatures{};
		deviceFeatures.samplerAnisotropy = VK_TRUE;

		VkDeviceCreateInfo createInfo{};
		createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
		createInfo.pQueueCreateInfos = queueCreateInfos.data();
		createInfo.enabledExtensionCount = 1;
		const char* deviceExtensions[] = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		createInfo.ppEnabledExtensionNames = deviceExtensions;
		createInfo.pEnabledFeatures = &deviceFeatures;
		AX_CORE_ASSERT(vkCreateDevice(adapter.getHandle(), &createInfo, nullptr, &device) == VK_SUCCESS, "Failed to create logical device!");
	
		mainQueue = createQueue(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_COMPUTE_BIT);
		if (mainQueue) {
			queues[VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_COMPUTE_BIT] = mainQueue.get();
			queues[VK_QUEUE_GRAPHICS_BIT] = mainQueue.get();
			queues[VK_QUEUE_TRANSFER_BIT] = mainQueue.get();
			queues[VK_QUEUE_COMPUTE_BIT] = mainQueue.get();
		} else {
			AX_CORE_LOG_ERROR("Failed to create Main Vulkan Queue!");
		}
	}

	std::unique_ptr<Context> VulkanDevice::createContext() {
		std::unique_ptr<VulkanContext> context = std::make_unique<VulkanContext>(*this, *getQueue(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_COMPUTE_BIT));
		context->init(3);
		return std::move(context);
	}

	std::unique_ptr<Swapchain> VulkanDevice::createSwapchain(SwapchainCreateInfo& swapchainCreateInfo) {
		std::unique_ptr<VulkanSwapchain> swapchain = std::make_unique<VulkanSwapchain>(swapchainCreateInfo, *this, *getQueue(VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT | VK_QUEUE_COMPUTE_BIT));
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

	std::unique_ptr<VulkanQueue> VulkanDevice::createQueue(VkQueueFlags flags) {
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
