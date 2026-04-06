#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION
#include "MetalDevice.h"
#include "Core/Assert.h"
#include "Platform/MacOS/MacOSWindow.h"

namespace Axiom {
    MetalDevice::MetalDevice(const CreateInfo &createInfo) : Device() {
        metalDevice = MTL::CreateSystemDefaultDevice();
        metalLayer = CA::MetalLayer::layer();
        AX_CORE_ASSERT(metalDevice, "Failed to create Metal device");
        AX_CORE_ASSERT(metalLayer, "Failed to create Metal layer");

        metalLayer->setDevice(metalDevice);
        metalLayer->setPixelFormat(MTL::PixelFormatBGRA8Unorm);
        reinterpret_cast<MacOSWindow *>(createInfo.windowObjPtr)->attachMetalLayer(metalLayer);

        commandQueue = metalDevice->newCommandQueue();
        AX_CORE_ASSERT(commandQueue, "Failed to create Metal command queue");
    }

    MetalDevice::~MetalDevice() {
        if (metalDevice) {
            metalDevice->release();
            metalDevice = nullptr;
        }
        if (metalLayer) {
            metalLayer->release();
            metalLayer = nullptr;
        }
        if (commandQueue) {
            commandQueue->release();
            commandQueue = nullptr;
        }
    }

    std::unique_ptr<SwapChain> MetalDevice::createSwapchain(uint32_t width, uint32_t height) {
        return std::make_unique<MetalSwapChain>(metalDevice, metalLayer, width, height);
    }

    std::unique_ptr<Pipeline> MetalDevice::createPipeline(const Pipeline::CreateInfo &pipelineCreateInfo) {
        return std::make_unique<MetalPipeline>(pipelineCreateInfo, metalDevice);
    }

    std::unique_ptr<CommandBuffer> MetalDevice::createCommandBuffer() {
        return std::make_unique<MetalCommandBuffer>(commandQueue);
    }

    std::unique_ptr<Semaphore> MetalDevice::createSemaphore() {
        return std::make_unique<MetalSemaphore>(metalDevice);
    }

    std::unique_ptr<Fence> MetalDevice::createFence(bool isSignaled) {
        return std::make_unique<MetalFence>(metalDevice, isSignaled);
    }

    std::unique_ptr<Buffer> MetalDevice::createBuffer(const Buffer::CreateInfo &bufferCreateInfo) {
        return std::make_unique<MetalBuffer>(bufferCreateInfo, metalDevice);
    }

    std::shared_ptr<Texture> MetalDevice::createTexture(const Texture::CreateInfo &textureCreateInfo) {
        return std::make_shared<MetalTexture>(textureCreateInfo, metalDevice);
    }

    std::unique_ptr<Sampler> MetalDevice::createSampler(const Sampler::CreateInfo &samplerCreateInfo) {
        return std::make_unique<MetalSampler>(samplerCreateInfo, metalDevice);
    }
} // namespace Axiom
