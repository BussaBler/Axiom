#pragma once
#include "Device.h"

namespace Axiom {
    class Renderer {
      public:
        Renderer(Window* windowPtr);
        ~Renderer();

        void waitIdle();

        CommandBuffer* beginFrame();
        void endFrame();

        std::unique_ptr<Pipeline> createPipeline(const Pipeline::CreateInfo& pipelineCreateInfo);
        std::unique_ptr<Buffer> createBuffer(const Buffer::CreateInfo& bufferCreateInfo);
        std::shared_ptr<Texture> createTexture(const Texture::CreateInfo& textureCreateInfo);
        std::unique_ptr<Sampler> createSampler(const Sampler::CreateInfo& samplerCreateInfo);
        std::unique_ptr<ResourceLayout> createResourceLayout(const std::vector<ResourceLayout::BindingCreateInfo>& bindings);

        std::unique_ptr<CommandBuffer> beginSingleTimeCommands();
        void endSingleTimeCommands(CommandBuffer* commandBuffer);

        inline Texture* getCurrentRenderTarget() { return swapChain->getCurrentTexture(); }
        inline Format getRenderTargetFormat() { return swapChain->getTextureFormat(); }
        inline Math::uVec2 getCurrentRenderTargetSize() { return {swapChain->getWidth(), swapChain->getHeight()}; }
        inline Texture* getDefaultTexture() { return defaultTexture.get(); }
        inline Sampler* getLinearSampler() { return linearSampler.get(); }
        inline Sampler* getNearestSampler() { return nearestSampler.get(); }

        void recreateSwapChain();

      private:
        void createDefaultTexture();
        void createDefaultSamplers();

      private:
        // TODO: remove this pointer handle the resizes through the event system
        Window* window = nullptr;
        std::unique_ptr<Device> device = nullptr;
        std::unique_ptr<SwapChain> swapChain = nullptr;
        Texture::Barrier renderTargetBarrier;
        Texture::Barrier presentBarrier;
        std::shared_ptr<Texture> defaultTexture = nullptr;
        std::unique_ptr<Sampler> linearSampler = nullptr;
        std::unique_ptr<Sampler> nearestSampler = nullptr;
    };
} // namespace Axiom
