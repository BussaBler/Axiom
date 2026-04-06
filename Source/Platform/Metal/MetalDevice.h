#pragma once
#include "MetalBuffer.h"
#include "MetalCommandBuffer.h"
#include "MetalFence.h"
#include "MetalPipeline.h"
#include "MetalSampler.h"
#include "MetalSemaphore.h"
#include "MetalSwapChain.h"
#include "MetalTexture.h"
#include "MetalUtils.h"
#include "Renderer/Device.h"

namespace Axiom {
    class MetalDevice : public Device {
      public:
        MetalDevice(const CreateInfo& createInfo);
        ~MetalDevice() override;

        std::unique_ptr<SwapChain> createSwapchain(uint32_t width, uint32_t height) override;
        std::unique_ptr<Pipeline> createPipeline(const Pipeline::CreateInfo& pipelineCreateInfo) override;
        std::unique_ptr<CommandBuffer> createCommandBuffer() override;
        std::unique_ptr<Semaphore> createSemaphore() override;
        std::unique_ptr<Fence> createFence(bool isSignaled) override;
        std::unique_ptr<Buffer> createBuffer(const Buffer::CreateInfo& bufferCreateInfo) override;
        std::shared_ptr<Texture> createTexture(const Texture::CreateInfo& textureCreateInfo) override;
        std::unique_ptr<Sampler> createSampler(const Sampler::CreateInfo& samplerCreateInfo) override;
        std::unique_ptr<ResourceLayout> createResourceLayout(const std::vector<ResourceLayout::BindingCreateInfo>& bindings) override;
        std::unique_ptr<CommandBuffer> beginSingleTimeCommands() override;
        void endSingleTimeCommands(CommandBuffer* commandBuffer) override;
        void submitCommandBuffers(const std::vector<CommandBuffer*> commandBuffers, const std::vector<Semaphore*> waitSemaphores,
                                  const std::vector<Semaphore*> signalSemaphores, Fence* signalFence) override;
        void waitIdle() override;

      private:
        MTL::Device* metalDevice = nullptr;
        CA::MetalLayer* metalLayer = nullptr;
        MTL::CommandQueue* commandQueue = nullptr;
    };
} // namespace Axiom