#include "VulkanDevice.h"

VULKAN_HPP_DEFAULT_DISPATCH_LOADER_DYNAMIC_STORAGE

namespace Axiom {
    static VKAPI_ATTR Vk::Bool32 VKAPI_CALL debugCallback(Vk::DebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                                          Vk::DebugUtilsMessageTypeFlagsEXT messageType,
                                                          const Vk::DebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        switch (messageSeverity) {
        case Vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose:
            AX_CORE_LOG_TRACE("Vulkan validation layer: {0}", pCallbackData->pMessage);
            break;
        case Vk::DebugUtilsMessageSeverityFlagBitsEXT::eInfo:
            AX_CORE_LOG_INFO("Vulkan validation layer: {0}", pCallbackData->pMessage);
            break;
        case Vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning:
            AX_CORE_LOG_WARN("Vulkan validation layer: {0}", pCallbackData->pMessage);
            break;
        case Vk::DebugUtilsMessageSeverityFlagBitsEXT::eError:
            AX_CORE_LOG_ERROR("Vulkan validation layer: {0}", pCallbackData->pMessage);
            break;
        default:
            break;
        }
        return Vk::False;
    }

    VulkanDevice::VulkanDevice(const Device::CreateInfo& createInfo) {
        AX_CORE_LOG_INFO("Initializing Vulkan Device...");
        AX_CORE_ASSERT(linkVulkanLib(), "Failed to link Vulkan library!");
        VULKAN_HPP_DEFAULT_DISPATCHER.init(vkInstanceProcAddr);

        Vk::ApplicationInfo appInfo(
            createInfo.appName.c_str(), Vk::makeVersion(createInfo.appVersionMajor, createInfo.appVersionMinor, createInfo.appVersionPatch),
            createInfo.engineName.c_str(), Vk::makeVersion(createInfo.engineVersionMajor, createInfo.engineVersionMinor, createInfo.engineVersionPatch),
            Vk::ApiVersion14);

        std::vector<const char*> extensions = getRequiredExtensions();
        std::vector<const char*> layers = getValidationLayers();

        Vk::InstanceCreateInfo instanceCreateInfo({}, &appInfo, layers, extensions);
        Vk::ResultValue<Vk::Instance> instanceResult = Vk::createInstance(instanceCreateInfo, nullptr, VULKAN_HPP_DEFAULT_DISPATCHER);

        AX_CORE_ASSERT(instanceResult.result == Vk::Result::eSuccess, "Failed to create Vulkan instance: {}", Vk::to_string(instanceResult.result));
        instance = instanceResult.value;
        VULKAN_HPP_DEFAULT_DISPATCHER.init(instance);

#if defined(AX_DEBUG)
        createDebugMessenger();
#endif
        createSurface(createInfo.windowObjPtr);
        pickPhysicalDevice();
        createLogicalDevice();
        VULKAN_HPP_DEFAULT_DISPATCHER.init(logicalDevice);
        createCommandPool();
        maxFramesInFlight = createInfo.maxFramesInFlight;
        commandBuffers.reserve(maxFramesInFlight);
        inFlightFences.reserve(maxFramesInFlight);
        for (size_t i = 0; i < maxFramesInFlight; i++) {
            commandBuffers.push_back(std::make_unique<VulkanCommandBuffer>(logicalDevice, commandPool));
            Vk::FenceCreateInfo fenceCreateInfo({}, 0);
            Vk::ResultValue<Vk::Fence> fenceResult = logicalDevice.createFence(fenceCreateInfo, nullptr);
            AX_CORE_ASSERT(fenceResult.result == Vk::Result::eSuccess, "Failed to create Vulkan fence: {}", Vk::to_string(fenceResult.result));
            inFlightFences.push_back(fenceResult.value);
        }
        createDescriptorPool();
        VulkanAllocator::init(logicalDevice, physicalDevice);
    }

    VulkanDevice::~VulkanDevice() {
        waitIdle();
        VulkanAllocator::shutdown();
        if (descriptorPool) {
            logicalDevice.destroyDescriptorPool(descriptorPool);
        }
        if (commandPool) {
            logicalDevice.destroyCommandPool(commandPool);
        }
        if (logicalDevice) {
            logicalDevice.destroy();
        }
        if (surface) {
            instance.destroySurfaceKHR(surface);
        }
        if (debugMessenger) {
            instance.destroyDebugUtilsMessengerEXT(debugMessenger);
        }
        if (instance) {
            instance.destroy();
        }
    }

    std::unique_ptr<SwapChain> VulkanDevice::createSwapchain(uint32_t width, uint32_t height) {
        SwapChainSupportDetails swapChainSupport = querySwapChainSupport(physicalDevice);
        QueueFamilyIndices queueFamilyIndices = findQueueFamilies(physicalDevice);
        VulkanSwapChain::CreateInfo createInfo{
            logicalDevice, surface, presentQueue, swapChainSupport, queueFamilyIndices, Vk::Extent2D(width, height), findDepthFormat(), maxFramesInFlight};
        return std::make_unique<VulkanSwapChain>(createInfo);
    }

    std::unique_ptr<Shader> VulkanDevice::createShader(const std::string& vertexSource, const std::string& fragmentSource) {
        return std::make_unique<VulkanShader>(logicalDevice, vertexSource, fragmentSource);
    }

    std::unique_ptr<Pipeline> VulkanDevice::createPipeline(const Pipeline::CreateInfo& pipelineCreateInfo) {
        return std::make_unique<VulkanPipeline>(pipelineCreateInfo, logicalDevice, descriptorPool);
    }

    std::unique_ptr<CommandBuffer> VulkanDevice::createCommandBuffer() {
        return std::make_unique<VulkanCommandBuffer>(logicalDevice, commandPool);
    }

    std::unique_ptr<Buffer> VulkanDevice::createBuffer(const Buffer::CreateInfo& bufferCreateInfo) {
        return std::make_unique<VulkanBuffer>(logicalDevice, bufferCreateInfo);
    }

    std::unique_ptr<Texture> VulkanDevice::createTexture(const Texture::CreateInfo& textureCreateInfo) {
        return std::make_unique<VulkanTexture>(logicalDevice, textureCreateInfo);
    }

    std::unique_ptr<Sampler> VulkanDevice::createSampler(const Sampler::CreateInfo& samplerCreateInfo) {
        return std::make_unique<VulkanSampler>(logicalDevice, samplerCreateInfo);
    }

    std::unique_ptr<ResourceLayout> VulkanDevice::createResourceLayout(const std::vector<ResourceLayout::BindingCreateInfo>& bindings) {
        return std::make_unique<VulkanResourceLayout>(logicalDevice, bindings);
    }

    bool VulkanDevice::beginFrame(SwapChain* swapChain) {
        logicalDevice.waitForFences(1, &inFlightFences[currentFrameIndex], Vk::True, std::numeric_limits<uint64_t>::max());

        if (!swapChain->acquireNextImage()) {
            return false;
        }

        logicalDevice.resetFences(1, &inFlightFences[currentFrameIndex]);
        return true;
    }

    CommandBuffer* VulkanDevice::getCurrentCommandBuffer() {
        return commandBuffers[currentFrameIndex].get();
    }

    std::unique_ptr<CommandBuffer> VulkanDevice::beginSingleTimeCommands() {
        VulkanCommandBuffer* commandBuffer = new VulkanCommandBuffer(logicalDevice, commandPool);
        commandBuffer->begin();

        return std::make_unique<VulkanCommandBuffer>(*commandBuffer);
    }

    void VulkanDevice::endSingleTimeCommands(CommandBuffer* commandBuffer) {
        commandBuffer->end();
        std::vector<CommandBuffer*> commandBuffers = {commandBuffer};
        std::array<Vk::CommandBuffer, 1> vkCommandBuffers = {static_cast<VulkanCommandBuffer*>(commandBuffer)->getHandle()};

        Vk::SubmitInfo submitInfo({}, {}, vkCommandBuffers, {});
        AX_CORE_ASSERT(graphicsQueue.submit(submitInfo, nullptr) == Vk::Result::eSuccess,
                       "Failed to submit single time command buffer to Vulkan graphics queue: {}", Vk::to_string(graphicsQueue.submit(submitInfo, nullptr)));
        AX_CORE_ASSERT(graphicsQueue.waitIdle() == Vk::Result::eSuccess,
                       "Failed to wait for Vulkan graphics queue idle after submitting single time command buffer: {}",
                       Vk::to_string(graphicsQueue.waitIdle()));
    }

    void VulkanDevice::submitCommandBuffers(const std::vector<CommandBuffer*> commandBuffers, SwapChain* swapChain) {
        std::vector<Vk::CommandBuffer> vkCommandBuffers(commandBuffers.size());
        for (size_t i = 0; i < commandBuffers.size(); i++) {
            vkCommandBuffers[i] = static_cast<VulkanCommandBuffer*>(commandBuffers[i])->getHandle();
        }

        std::array<Vk::Semaphore, 1> vkWaitSemaphores = {static_cast<VulkanSwapChain*>(swapChain)->getImageAvailableSemaphore(currentFrameIndex)};
        std::array<Vk::Semaphore, 1> vkSignalSemaphores = {static_cast<VulkanSwapChain*>(swapChain)->getRenderFinishedSemaphore()};
        Vk::Fence vkSignalFence = inFlightFences[currentFrameIndex];

        std::array<Vk::PipelineStageFlags, 1> waitStages = {Vk::PipelineStageFlagBits::eColorAttachmentOutput};

        Vk::SubmitInfo submitInfo(vkWaitSemaphores, waitStages, vkCommandBuffers, vkSignalSemaphores);

        AX_CORE_ASSERT(graphicsQueue.submit(submitInfo, vkSignalFence) == Vk::Result::eSuccess, "Failed to submit command buffers to Vulkan graphics queue: {}",
                       Vk::to_string(graphicsQueue.submit(submitInfo, vkSignalFence)));

        currentFrameIndex = (currentFrameIndex + 1) % maxFramesInFlight;
    }

    void VulkanDevice::waitIdle() {
        AX_CORE_ASSERT(logicalDevice.waitIdle() == Vk::Result::eSuccess, "Failed to wait for Vulkan device idle: {}", Vk::to_string(logicalDevice.waitIdle()));
    }

    bool VulkanDevice::linkVulkanLib() {
#if defined(AX_PLATFORM_WINDOWS)
        HMODULE vulkanLib = LoadLibraryA("vulkan-1.dll");
        if (vulkanLib == nullptr) {
            AX_CORE_LOG_ERROR("Failed to load Vulkan library!");
            return false;
        }
        vkInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(GetProcAddress(vulkanLib, "vkGetInstanceProcAddr"));
#elif defined(AX_PLATFORM_LINUX)
        void* vulkanLib = dlopen("libvulkan.so.1", RTLD_NOW | RTLD_LOCAL);
        if (vulkanLib == nullptr) {
            AX_CORE_LOG_ERROR("Failed to load Vulkan library: {}", dlerror());
            return false;
        }
        vkInstanceProcAddr = reinterpret_cast<PFN_vkGetInstanceProcAddr>(dlsym(vulkanLib, "vkGetInstanceProcAddr"));
#endif
        return vkInstanceProcAddr != nullptr;
    }

    std::vector<const char*> VulkanDevice::getRequiredExtensions() {
        std::vector<const char*> extensions{};
        extensions.push_back(Vk::KHRSurfaceExtensionName);
#if defined(AX_DEBUG)
        extensions.push_back(Vk::EXTDebugUtilsExtensionName);
#endif
#if defined(AX_PLATFORM_WINDOWS)
        extensions.push_back(Vk::KHRWin32SurfaceExtensionName);
#elif defined(AX_PLATFORM_LINUX)
        extensions.push_back(Vk::KHRXlibSurfaceExtensionName);
#endif
        for (const char* ext : extensions) {
            AX_CORE_LOG_DEBUG("Required Vulkan Device Extension: {}", ext);
        }
        return extensions;
    }

    std::vector<const char*> VulkanDevice::getValidationLayers() {
        std::vector<const char*> layers{};
#if defined(AX_DEBUG)
        layers.push_back("VK_LAYER_KHRONOS_validation");
#endif
        Vk::ResultValue<std::vector<Vk::LayerProperties>> availableLayersResult = Vk::enumerateInstanceLayerProperties();

        AX_CORE_ASSERT(availableLayersResult.result == Vk::Result::eSuccess, "Failed to enumerate Vulkan instance layers: {}",
                       Vk::to_string(availableLayersResult.result));
        std::vector<Vk::LayerProperties> availableLayers = availableLayersResult.value;

        for (size_t i = 0; i < layers.size(); i++) {
            bool layerFound = false;
            for (const auto& layer : availableLayers) {
                if (layers[i] == std::string_view(layer.layerName.data())) {
                    layerFound = true;
                    break;
                }
            }
            AX_CORE_ASSERT(layerFound, "Validation layer not found: {}", layers[i]);
        }

        return layers;
    }

    void VulkanDevice::createDebugMessenger() {
        Vk::DebugUtilsMessengerCreateInfoEXT debugMessengerCreateInfo(
            {},
            Vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose | Vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
                Vk::DebugUtilsMessageSeverityFlagBitsEXT::eError,
            Vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | Vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation |
                Vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance,
            debugCallback);
        Vk::ResultValue<Vk::DebugUtilsMessengerEXT> debugMessengerResult = instance.createDebugUtilsMessengerEXT(debugMessengerCreateInfo);

        AX_CORE_ASSERT(debugMessengerResult.result == Vk::Result::eSuccess, "Failed to create Vulkan debug messenger: {}",
                       Vk::to_string(debugMessengerResult.result));
        debugMessenger = debugMessengerResult.value;
    }

    void VulkanDevice::createSurface(Window* windowObjPtr) {
#if defined(AX_PLATFORM_WINDOWS)
        Win32Window* win32Window = reinterpret_cast<Win32Window*>(windowObjPtr);

        Vk::Win32SurfaceCreateInfoKHR surfaceCreateInfo({}, reinterpret_cast<HINSTANCE>(win32Window->getNativeDisplay()),
                                                        reinterpret_cast<HWND>(win32Window->getNativeWindow()));
        Vk::ResultValue<Vk::SurfaceKHR> surfaceResult = instance.createWin32SurfaceKHR(surfaceCreateInfo);

        AX_CORE_ASSERT(surfaceResult.result == Vk::Result::eSuccess, "Failed to create Vulkan Win32 surface: {}", Vk::to_string(surfaceResult.result));
        surface = surfaceResult.value;
#elif defined(AX_PLATFORM_LINUX)
        XLibWindow* xLibWindow = reinterpret_cast<XLibWindow*>(windowObjPtr);

        Vk::XlibSurfaceCreateInfoKHR createInfo({}, static_cast<Display*>(xLibWindow->getNativeDisplay()),
                                                *static_cast<::Window*>(xLibWindow->getNativeWindow()));
        Vk::ResultValue<Vk::SurfaceKHR> surfaceResult = instance.createXlibSurfaceKHR(createInfo);

        AX_CORE_ASSERT(surfaceResult.result == Vk::Result::eSuccess, "Failed to create window surface!");
        surface = surfaceResult.value;
#endif
    }

    void VulkanDevice::pickPhysicalDevice() {
        Vk::ResultValue<std::vector<Vk::PhysicalDevice>> physicalDevicesResult = instance.enumeratePhysicalDevices();

        AX_CORE_ASSERT(physicalDevicesResult.result == Vk::Result::eSuccess, "Failed to enumerate Vulkan physical devices: {}",
                       Vk::to_string(physicalDevicesResult.result));
        std::vector<Vk::PhysicalDevice> physicalDevices = physicalDevicesResult.value;

        for (const auto& device : physicalDevices) {
            Vk::PhysicalDeviceProperties deviceProperties = device.getProperties();
            AX_CORE_LOG_DEBUG("Found Vulkan Adapter: {} (API Version: {}.{}.{} | Driver Version: {} | Vendor ID: {} | Device ID: {})",
                              std::string_view(deviceProperties.deviceName.data()), Vk::versionMajor(deviceProperties.apiVersion),
                              Vk::versionMinor(deviceProperties.apiVersion), Vk::versionPatch(deviceProperties.apiVersion), deviceProperties.driverVersion,
                              deviceProperties.vendorID, deviceProperties.deviceID);

            bool extensionsSupported = checkPhysicalDeviceExtensions(device);
            QueueFamilyIndices indices = findQueueFamilies(device);
            bool swapChainAdequate = false;
            if (extensionsSupported) {
                SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
                swapChainAdequate = !swapChainSupport.formats.empty() && !swapChainSupport.presentModes.empty();
            }

            if (extensionsSupported && indices.isComplete() && swapChainAdequate) {
                AX_CORE_LOG_DEBUG("Selected Vulkan Adapter: {}", std::string_view(deviceProperties.deviceName.data()));
                physicalDevice = device;
                break;
            }
        }
    }

    bool VulkanDevice::checkPhysicalDeviceExtensions(const Vk::PhysicalDevice& device) {
        Vk::ResultValue<std::vector<Vk::ExtensionProperties>> availableExtensionsResult = device.enumerateDeviceExtensionProperties();

        AX_CORE_ASSERT(availableExtensionsResult.result == Vk::Result::eSuccess, "Failed to enumerate device extensions for device: {}",
                       std::string_view(device.getProperties().deviceName));
        std::vector<Vk::ExtensionProperties> availableExtensions = availableExtensionsResult.value;
        std::vector<const char*> requiredExtensions = {Vk::KHRSwapchainExtensionName, Vk::KHRDynamicRenderingExtensionName,
                                                       Vk::KHRSynchronization2ExtensionName};

        for (const char* requiredExt : requiredExtensions) {
            bool extensionFound = false;
            for (const auto& ext : availableExtensions) {
                if (requiredExt == std::string_view(ext.extensionName.data())) {
                    extensionFound = true;
                    break;
                }
            }
            if (!extensionFound) {
                AX_CORE_LOG_WARN("Physical device {} is missing required extension: {}", std::string_view(device.getProperties().deviceName), requiredExt);
                return false;
            }
        }

        return true;
    }

    QueueFamilyIndices VulkanDevice::findQueueFamilies(const Vk::PhysicalDevice& device) const {
        QueueFamilyIndices indices{};

        std::vector<Vk::QueueFamilyProperties> queueFamilies = device.getQueueFamilyProperties();
        for (size_t i = 0; i < queueFamilies.size(); i++) {
            if (queueFamilies[i].queueFlags & Vk::QueueFlagBits::eGraphics) {
                indices.graphicsFamily = static_cast<uint32_t>(i);
            }

            Vk::ResultValue<Vk::Bool32> presentSupportResult = device.getSurfaceSupportKHR(i, surface);
            AX_CORE_ASSERT(presentSupportResult.result == Vk::Result::eSuccess, "Failed to query surface support for device: {}",
                           std::string_view(device.getProperties().deviceName));

            if (presentSupportResult.value) {
                indices.presentFamily = static_cast<uint32_t>(i);
            }

            if (indices.isComplete()) {
                break;
            }
        }

        return indices;
    }

    SwapChainSupportDetails VulkanDevice::querySwapChainSupport(const Vk::PhysicalDevice& device) const {
        SwapChainSupportDetails details{};

        Vk::ResultValue<Vk::SurfaceCapabilitiesKHR> capabilitiesResult = device.getSurfaceCapabilitiesKHR(surface);

        AX_CORE_ASSERT(capabilitiesResult.result == Vk::Result::eSuccess, "Failed to query surface capabilities for device: {}",
                       std::string_view(device.getProperties().deviceName));
        details.capabilities = capabilitiesResult.value;

        Vk::ResultValue<std::vector<Vk::SurfaceFormatKHR>> formatsResult = device.getSurfaceFormatsKHR(surface);

        AX_CORE_ASSERT(formatsResult.result == Vk::Result::eSuccess, "Failed to query surface formats for device: {}",
                       std::string_view(device.getProperties().deviceName));
        details.formats = formatsResult.value;

        Vk::ResultValue<std::vector<Vk::PresentModeKHR>> presentModesResult = device.getSurfacePresentModesKHR(surface);
        AX_CORE_ASSERT(presentModesResult.result == Vk::Result::eSuccess, "Failed to query surface present modes for device: {}",
                       std::string_view(device.getProperties().deviceName));
        details.presentModes = presentModesResult.value;

        return details;
    }

    Vk::Format VulkanDevice::findDepthFormat() const {
        std::array<Vk::Format, 3> depthFormats = {Vk::Format::eD32Sfloat, Vk::Format::eD32SfloatS8Uint, Vk::Format::eD24UnormS8Uint};
        for (Vk::Format format : depthFormats) {
            Vk::FormatProperties formatProperties = physicalDevice.getFormatProperties(format);
            if ((formatProperties.optimalTilingFeatures & Vk::FormatFeatureFlagBits::eDepthStencilAttachment) != Vk::FormatFeatureFlags()) {
                return format;
            }
        }

        AX_CORE_ASSERT(false, "Failed to find supported depth format");
        return Vk::Format::eD32Sfloat;
    }

    void VulkanDevice::createLogicalDevice() {
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

        std::vector<Vk::DeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};
        std::array<float, 1> queuePriorities = {1.0f};

        for (uint32_t queueFamilyIndex : uniqueQueueFamilies) {
            queueCreateInfos.emplace_back(Vk::DeviceQueueCreateFlags{}, queueFamilyIndex, queuePriorities);
        }

        Vk::PhysicalDeviceFeatures deviceFeatures{};
        deviceFeatures.setSamplerAnisotropy(Vk::True);
        Vk::PhysicalDeviceDynamicRenderingFeatures dynamicRenderingFeatures{};
        dynamicRenderingFeatures.setDynamicRendering(Vk::True);
        Vk::PhysicalDeviceSynchronization2Features synchronization2Features{};
        synchronization2Features.setSynchronization2(Vk::True);

        dynamicRenderingFeatures.setPNext(&synchronization2Features);

        Vk::DeviceCreateInfo deviceCreateInfo({}, queueCreateInfos, {}, {Vk::KHRSwapchainExtensionName}, &deviceFeatures, &dynamicRenderingFeatures);
        Vk::ResultValue<Vk::Device> logicalDeviceResult = physicalDevice.createDevice(deviceCreateInfo);

        AX_CORE_ASSERT(logicalDeviceResult.result == Vk::Result::eSuccess, "Failed to create Vulkan logical device: {}",
                       Vk::to_string(logicalDeviceResult.result));
        logicalDevice = logicalDeviceResult.value;

        graphicsQueue = logicalDevice.getQueue(indices.graphicsFamily.value(), 0);
        presentQueue = logicalDevice.getQueue(indices.presentFamily.value(), 0);
    }

    void VulkanDevice::createCommandPool() {
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);
        Vk::CommandPoolCreateInfo poolCreateInfo(Vk::CommandPoolCreateFlagBits::eResetCommandBuffer, indices.graphicsFamily.value());
        Vk::ResultValue<Vk::CommandPool> commandPoolResult = logicalDevice.createCommandPool(poolCreateInfo);

        AX_CORE_ASSERT(commandPoolResult.result == Vk::Result::eSuccess, "Failed to create Vulkan command pool: {}", Vk::to_string(commandPoolResult.result));
        commandPool = commandPoolResult.value;
    }

    void VulkanDevice::createDescriptorPool() {
        std::array<Vk::DescriptorPoolSize, 5> poolSizes = {
            Vk::DescriptorPoolSize(Vk::DescriptorType::eUniformBuffer, 100),       Vk::DescriptorPoolSize(Vk::DescriptorType::eStorageBuffer, 100),
            Vk::DescriptorPoolSize(Vk::DescriptorType::eSampledImage, 100),        Vk::DescriptorPoolSize(Vk::DescriptorType::eSampler, 100),
            Vk::DescriptorPoolSize(Vk::DescriptorType::eCombinedImageSampler, 50),
        };

        Vk::DescriptorPoolCreateInfo poolCreateInfo(Vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, 500, poolSizes);
        Vk::ResultValue<Vk::DescriptorPool> descriptorPoolResult = logicalDevice.createDescriptorPool(poolCreateInfo);

        AX_CORE_ASSERT(descriptorPoolResult.result == Vk::Result::eSuccess, "Failed to create Vulkan descriptor pool: {}",
                       Vk::to_string(descriptorPoolResult.result));
        descriptorPool = descriptorPoolResult.value;
    }
} // namespace Axiom
