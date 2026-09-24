#include "axpch.h"

#define NS_PRIVATE_IMPLEMENTATION
#define CA_PRIVATE_IMPLEMENTATION
#define MTL_PRIVATE_IMPLEMENTATION

#include "Core/Assert.h"
#include "Metal/Metal.hpp"
#include "MetalDevice.h"
#include "Platform/MacOS/MacOSWindow.h"

namespace Axiom {
    std::unique_ptr<Device> Device::create(const CreateInfo& createInfo) {
        return std::make_unique<MetalDevice>(createInfo);
    }

    MetalDevice::MetalDevice(const CreateInfo& createInfo) : Device() {
        metalDevice = MTL::CreateSystemDefaultDevice();
        metalLayer = static_cast<CA::MetalLayer*>(reinterpret_cast<MacOSWindow*>(createInfo.windowObjPtr)->getMetalLayer());
        AX_CORE_ASSERT(metalDevice, "Failed to create Metal device");
        AX_CORE_ASSERT(metalLayer, "Failed to get Metal layer");

        metalLayer->setDevice(metalDevice);
        metalLayer->setPixelFormat(MTL::PixelFormatBGRA8Unorm);

        commandQueue = metalDevice->newCommandQueue();
        AX_CORE_ASSERT(commandQueue, "Failed to create Metal command queue");

        maxFramesInFlight = createInfo.maxFramesInFlight;
        commandBuffers.reserve(maxFramesInFlight);
        for (uint32_t i = 0; i < maxFramesInFlight; i++) {
            commandBuffers.push_back(std::make_unique<MetalCommandBuffer>(commandQueue));
        }
        frameSemaphore = dispatch_semaphore_create(maxFramesInFlight);
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
        return std::make_unique<MetalSwapChain>(metalDevice, metalLayer, width, height, maxFramesInFlight);
    }

    std::unique_ptr<Shader> MetalDevice::createShader(const std::string& vertexSource, const std::string& fragmentSource) {
        return std::make_unique<MetalShader>(vertexSource, fragmentSource, metalDevice);
    }

    std::unique_ptr<Pipeline> MetalDevice::createPipeline(const Pipeline::CreateInfo& pipelineCreateInfo) {
        return std::make_unique<MetalPipeline>(pipelineCreateInfo, metalDevice);
    }

    std::unique_ptr<CommandBuffer> MetalDevice::createCommandBuffer() {
        return std::make_unique<MetalCommandBuffer>(commandQueue);
    }

    std::unique_ptr<Buffer> MetalDevice::createBuffer(const Buffer::CreateInfo& bufferCreateInfo) {
        return std::make_unique<MetalBuffer>(bufferCreateInfo, metalDevice);
    }

    std::unique_ptr<Texture> MetalDevice::createTexture(const Texture::CreateInfo& textureCreateInfo) {
        return std::make_unique<MetalTexture>(textureCreateInfo, metalDevice);
    }

    std::unique_ptr<Sampler> MetalDevice::createSampler(const Sampler::CreateInfo& samplerCreateInfo) {
        return std::make_unique<MetalSampler>(samplerCreateInfo, metalDevice);
    }

    std::unique_ptr<ResourceLayout> MetalDevice::createResourceLayout(const std::vector<ResourceLayout::BindingCreateInfo>& bindings) {
        return std::make_unique<MetalResourceLayout>(bindings);
    }

    std::unique_ptr<ResourceSet> MetalDevice::createResourceSet(const ResourceLayout* resourceLayout) {
        return std::make_unique<MetalResourceSet>(resourceLayout, metalDevice);
    }

    bool MetalDevice::beginFrame(SwapChain* swapChain) {
        dispatch_semaphore_wait(frameSemaphore, DISPATCH_TIME_FOREVER);
        static_cast<MetalSwapChain*>(swapChain)->setCurrentFrameIndex(currentFrameIndex);

        if (!swapChain->acquireNextImage()) {
            dispatch_semaphore_signal(frameSemaphore);
            return false;
        }
        return true;
    }

    CommandBuffer* MetalDevice::getCurrentCommandBuffer() {
        return commandBuffers[currentFrameIndex].get();
    }

    std::unique_ptr<CommandBuffer> MetalDevice::beginSingleTimeCommands() {
        std::unique_ptr<CommandBuffer> commandBuffer = createCommandBuffer();
        commandBuffer->begin();
        return std::move(commandBuffer);
    }

    void MetalDevice::endSingleTimeCommands(CommandBuffer* commandBuffer) {
        MetalCommandBuffer* metalCommandBuffer = static_cast<MetalCommandBuffer*>(commandBuffer);
        metalCommandBuffer->end();
        std::vector<CommandBuffer*> commandBuffers = {commandBuffer};
        submitCommandBuffers(commandBuffers, nullptr);
    }

    void MetalDevice::submitCommandBuffers(const std::vector<CommandBuffer*> commandBuffers, SwapChain* swapChain) {
        MetalSwapChain* mtlSwapChain = static_cast<MetalSwapChain*>(swapChain);

        for (size_t i = 0; i < commandBuffers.size(); ++i) {
            MetalCommandBuffer* metalCommandBuffer = static_cast<MetalCommandBuffer*>(commandBuffers[i]);
            MTL::CommandBuffer* mtlCommandBuffer = metalCommandBuffer->getHandle();

            bool isLast = (i == commandBuffers.size() - 1);

            if (isLast) {
                if (mtlSwapChain && mtlSwapChain->getCurrentDrawable()) {
                    mtlCommandBuffer->presentDrawable(mtlSwapChain->getCurrentDrawable());
                }
                mtlCommandBuffer->addCompletedHandler(^(MTL::CommandBuffer* cb) {
                    dispatch_semaphore_signal(frameSemaphore);
                });
            }

            mtlCommandBuffer->commit();
        }
        currentFrameIndex = (currentFrameIndex + 1) % maxFramesInFlight;
    }

    void MetalDevice::waitIdle() {
        // TODO
    }
} // namespace Axiom
