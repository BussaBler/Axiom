#pragma once
#include "Core/Assert.h"
#include "Renderer/Device.h"
#include "VulkanAllocator.h"
#include "VulkanBuffer.h"
#include "VulkanCommandBuffer.h"
#include "VulkanInclude.h"
#include "VulkanPipeline.h"
#include "VulkanResourceLayout.h"
#include "VulkanResourceSet.h"
#include "VulkanSampler.h"
#include "VulkanShader.h"
#include "VulkanSwapChain.h"
#include "VulkanTexture.h"
#include "VulkanUtils.h"
#include "axpch.h"

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
        std::shared_ptr<Shader> createShader(const std::string& vertexSource, const std::string& fragmentSource) override;
        std::unique_ptr<Pipeline> createPipeline(const Pipeline::CreateInfo& pipelineCreateInfo) override;
        std::unique_ptr<CommandBuffer> createCommandBuffer() override;
        std::unique_ptr<Buffer> createBuffer(const Buffer::CreateInfo& bufferCreateInfo) override;
        std::shared_ptr<Texture> createTexture(const Texture::CreateInfo& textureCreateInfo) override;
        std::unique_ptr<Sampler> createSampler(const Sampler::CreateInfo& samplerCreateInfo) override;
        std::unique_ptr<ResourceLayout> createResourceLayout(const std::vector<ResourceLayout::BindingCreateInfo>& bindings) override;
        bool beginFrame(SwapChain* swapChain) override;
        CommandBuffer* getCurrentCommandBuffer() override;
        std::unique_ptr<CommandBuffer> beginSingleTimeCommands() override;
        void endSingleTimeCommands(CommandBuffer* commandBuffer) override;
        void submitCommandBuffers(const std::vector<CommandBuffer*> commandBuffers, SwapChain* swapChain) override;
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
        Vk::Format findDepthFormat() const;
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
        uint32_t maxFramesInFlight = 0;
        uint32_t currentFrameIndex = 0;
        std::vector<std::unique_ptr<VulkanCommandBuffer>> commandBuffers;
        std::vector<Vk::Fence> inFlightFences;
        Vk::DescriptorPool descriptorPool = nullptr;
    };
} // namespace Axiom
