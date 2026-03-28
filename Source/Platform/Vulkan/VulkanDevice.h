#pragma once
#include "axpch.h"
#include "Core/Assert.h"
#include "Renderer/Device.h"
#include "VulkanAllocator.h"
#include "VulkanBuffer.h"
#include "VulkanCommandBuffer.h"
#include "VulkanFence.h"
#include "VulkanInclude.h"
#include "VulkanPipeline.h"
#include "VulkanResourceLayout.h"
#include "VulkanResourceSet.h"
#include "VulkanSampler.h"
#include "VulkanSwapChain.h"
#include "VulkanTexture.h"
#include "VulkanUtils.h"

#if defined(AX_PLATFORM_WINDOWS)
	#include "Platform/Windows/Win32Window.h"
#elif defined(AX_PLATFORM_LINUX)
	#include "Platform/Linux/XLibWindow.h"
#endif

namespace Axiom {
	class VulkanDevice : public Device {
	public:
		VulkanDevice(const Device::CreateInfo& createInfo);
		~VulkanDevice() override;

		std::unique_ptr<SwapChain> createSwapchain(uint32_t width, uint32_t height) override;
		std::unique_ptr<Pipeline> createPipeline(const Pipeline::CreateInfo& pipelineCreateInfo) override;
		std::unique_ptr<CommandBuffer> createCommandBuffer() override;
		std::unique_ptr<Semaphore> createSemaphore() override;
		std::unique_ptr<Fence> createFence(bool isSignaled) override;
		std::unique_ptr<Buffer> createBuffer(const Buffer::CreateInfo& bufferCreateInfo) override;
		std::shared_ptr<Texture> createTexture(const Texture::CreateInfo& textureCreateInfo) override;
		std::unique_ptr<Sampler> createSampler(const Sampler::CreateInfo& samplerCreateInfo) override;
		std::unique_ptr<ResourceLayout> createResourceLayout(const std::vector<ResourceLayout::BindingCreateInfo>& bindings) override;
		std::unique_ptr<ResourceSet> createResourceSet(ResourceLayout* resourceLayout) override;
		std::unique_ptr<CommandBuffer> beginSingleTimeCommands() override;
		void endSingleTimeCommands(CommandBuffer* commandBuffer) override;
		void submitCommandBuffers(const std::vector<CommandBuffer*> commandBuffers, const std::vector<Semaphore*> waitSemaphores, const std::vector<Semaphore*> signalSemaphores, Fence* signalFence) override;
		void waitIdle() override;

	private:
		bool linkVulkanLib();
		std::vector<const char*> getRequiredExtensions();
		std::vector<const char*> getValidationLayers();
		void createDebugMessenger();
		void createSurface(Window* windowObjPtr);
		void pickPhysicalDevice();
		static bool checkPhysicalDeviceExtensions(const Vk::PhysicalDevice& device);
		QueueFamilyIndices findQueueFamilies(const Vk::PhysicalDevice& device) const;
		SwapChainSupportDetails querySwapChainSupport(const Vk::PhysicalDevice& device) const;
		void createLogicalDevice();
		void createCommandPool();
		void createDescriptorPool();

	private:
		PFN_vkGetInstanceProcAddr vkInstanceProcAddr = nullptr;
		Vk::Instance instance = nullptr;
		Vk::DebugUtilsMessengerEXT debugMessenger = nullptr;
		Vk::SurfaceKHR surface = nullptr;
		Vk::PhysicalDevice physicalDevice = nullptr;
		Vk::Device logicalDevice = nullptr;
		Vk::Queue graphicsQueue = nullptr;
		Vk::Queue presentQueue = nullptr;
		Vk::CommandPool commandPool = nullptr;
		Vk::DescriptorPool descriptorPool = nullptr;
	};
}

