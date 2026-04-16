#include "Renderer.h"

namespace Axiom {
    Renderer::Renderer(Window* windowPtr) : window(windowPtr) {
        Device::CreateInfo deviceCreateInfo{};
        deviceCreateInfo.api = GraphicsApi::Vulkan;
        deviceCreateInfo.windowObjPtr = window;

        device = Device::create(deviceCreateInfo);
        swapChain = device->createSwapchain(window->getFramebufferWidth(), window->getFramebufferHeight());

        renderTargetBarrier = {.oldState = TextureState::Undefined, .newState = TextureState::RenderTarget, .aspect = TextureAspect::Color};
        presentBarrier = {.oldState = TextureState::RenderTarget, .newState = TextureState::Present, .aspect = TextureAspect::Color};

        createDefaultTexture();
        createDefaultSamplers();
    }

    Renderer::~Renderer() {
        device->waitIdle();
    }

    void Renderer::waitIdle() {
        device->waitIdle();
    }

    CommandBuffer* Renderer::beginFrame() {
        if (!device->beginFrame(swapChain.get())) {
            recreateSwapChain();
            return nullptr;
        }

        CommandBuffer* commandBuffer = device->getCurrentCommandBuffer();
        commandBuffer->begin();

        renderTargetBarrier.texture = swapChain->getCurrentTexture();
        commandBuffer->pipelineBarrier({renderTargetBarrier});
        return commandBuffer;
    }

    void Renderer::endFrame() {
        CommandBuffer* commandBuffer = device->getCurrentCommandBuffer();
        presentBarrier.texture = renderTargetBarrier.texture;
        commandBuffer->pipelineBarrier({presentBarrier});
        commandBuffer->end();

        device->submitCommandBuffers({commandBuffer}, swapChain.get());

        if (!swapChain->present()) {
            recreateSwapChain();
        }
    }

    std::unique_ptr<Pipeline> Renderer::createPipeline(const Pipeline::CreateInfo& pipelineCreateInfo) {
        return device->createPipeline(pipelineCreateInfo);
    }

    std::unique_ptr<Buffer> Renderer::createBuffer(const Buffer::CreateInfo& bufferCreateInfo) {
        return device->createBuffer(bufferCreateInfo);
    }

    std::shared_ptr<Texture> Renderer::createTexture(const Texture::CreateInfo& textureCreateInfo) {
        return device->createTexture(textureCreateInfo);
    }

    std::unique_ptr<Sampler> Renderer::createSampler(const Sampler::CreateInfo& samplerCreateInfo) {
        return device->createSampler(samplerCreateInfo);
    }

    std::unique_ptr<ResourceLayout> Renderer::createResourceLayout(const std::vector<ResourceLayout::BindingCreateInfo>& bindings) {
        return device->createResourceLayout(bindings);
    }

    std::unique_ptr<CommandBuffer> Renderer::beginSingleTimeCommands() {
        return device->beginSingleTimeCommands();
    }

    void Renderer::endSingleTimeCommands(CommandBuffer* commandBuffer) {
        device->endSingleTimeCommands(commandBuffer);
    }

    void Renderer::recreateSwapChain() {
        uint32_t width = window->getFramebufferWidth();
        uint32_t height = window->getFramebufferHeight();

        device->waitIdle();
        swapChain.reset();
        swapChain = device->createSwapchain(width, height);
    }

    void Renderer::createDefaultTexture() {
        const uint32_t defaultTextureSize = 4;

        Texture::CreateInfo textureCreateInfo = {.width = defaultTextureSize,
                                                 .height = defaultTextureSize,
                                                 .mipLevels = 1,
                                                 .arrayLayers = 1,
                                                 .format = Format::R8G8B8A8Unorm,
                                                 .usage = TextureUsage::Sampled | TextureUsage::TransferDst,
                                                 .aspect = TextureAspect::Color,
                                                 .initialState = TextureState::Undefined,
                                                 .memoryUsage = MemoryUsage::GPUOnly};
        defaultTexture = device->createTexture(textureCreateInfo);

        Buffer::CreateInfo stagingBufferCreateInfo = {
            .size = defaultTextureSize * defaultTextureSize * sizeof(uint32_t), .usage = BufferUsage::TransferSrc, .memoryUsage = MemoryUsage::GPUandCPU};
        std::unique_ptr<Buffer> stagingBuffer = device->createBuffer(stagingBufferCreateInfo);

        // Magenta: R=255, G=0, B=255, A=255
        // Black:   R=0,   G=0, B=0,   A=255
        const uint32_t magenta = 0xFF00FFFF; // AABBGGRR (check your API's expected byte order)
        const uint32_t black = 0xFF000000;

        std::array<uint32_t, defaultTextureSize * defaultTextureSize> defaultTextureData;
        for (uint32_t y = 0; y < defaultTextureSize; y++) {
            for (uint32_t x = 0; x < defaultTextureSize; x++) {
                if ((x + y) % 2 == 0) {
                    defaultTextureData[y * defaultTextureSize + x] = magenta;
                } else {
                    defaultTextureData[y * defaultTextureSize + x] = black;
                }
            }
        }

        std::unique_ptr<CommandBuffer> commandBuffer = device->beginSingleTimeCommands();
        stagingBuffer->setData<uint32_t>(defaultTextureData);
        commandBuffer->copyBufferToTexture(stagingBuffer.get(), defaultTexture.get(), defaultTextureSize, defaultTextureSize);
        device->endSingleTimeCommands(commandBuffer.get());
    }

    void Renderer::createDefaultSamplers() {
        Sampler::CreateInfo linearSamplerCreateInfo = {
            .adressMode = SamplerAddressMode::Repeat, .filterMode = SamplerFilterMode::Linear, .mipmapFilterMode = SamplerFilterMode::Linear};
        linearSampler = device->createSampler(linearSamplerCreateInfo);

        Sampler::CreateInfo nearestSamplerCreateInfo = {
            .adressMode = SamplerAddressMode::Repeat, .filterMode = SamplerFilterMode::Nearest, .mipmapFilterMode = SamplerFilterMode::Nearest};
        nearestSampler = device->createSampler(nearestSamplerCreateInfo);
    }
} // namespace Axiom
