#include "axpch.h"

#include "RenderHardwareInterface.h"

#include "Renderer/ForwardRenderPipeline.h"
#include "Renderer/Pipeline.h"
#include "Renderer/ResourceLayout.h"
#include "Renderer/ResourceSet.h"
#include "Renderer/Texture.h"
#include "TextureRegistry.h"

#include <memory>
#include <utility>

namespace Axiom {
    RenderHardwareInterface::RenderHardwareInterface(Window* windowPtr) : window(windowPtr) {
        Device::CreateInfo deviceCreateInfo{};
        // TODO: make the api represent the correct thing
        deviceCreateInfo.api = GraphicsApi::Vulkan;
        deviceCreateInfo.windowObjPtr = window;
        deviceCreateInfo.maxFramesInFlight = MAX_FRAMES_IN_FLIGHT;

        device = Device::create(deviceCreateInfo);
        swapChain = device->createSwapchain(Format::B8G8R8A8Unorm, window->getFramebufferWidth(), window->getFramebufferHeight());

        renderTargetBarrier = {.oldState = TextureState::Undefined, .newState = TextureState::RenderTarget, .aspect = TextureAspect::Color};
        presentBarrier = {.oldState = TextureState::RenderTarget, .newState = TextureState::Present, .aspect = TextureAspect::Color};
        depthBarrier = {.oldState = TextureState::Undefined, .newState = TextureState::DepthStencilTarget, .aspect = TextureAspect::Depth};

        createDefaultSamplers();

        textureRegistry = std::make_unique<TextureRegistry>(this);
        textureRegistry->init();
    }

    RenderHardwareInterface::~RenderHardwareInterface() {
        device->waitIdle();
    }

    void RenderHardwareInterface::initPipelines() {
        forwardRP = registerPipeline<ForwardRenderPipeline>(FORWARD_PIPELINE_NAME);
    }

    void RenderHardwareInterface::waitIdle() {
        device->waitIdle();
    }

    CommandBuffer* RenderHardwareInterface::beginFrame() {
        for (auto& [name, pipeline] : registeredPipelines) {
            pipeline->beginFrame();
        }

        if (!device->beginFrame(swapChain.get())) {
            recreateSwapChain();
            return nullptr;
        }

        CommandBuffer* commandBuffer = device->getCurrentCommandBuffer();
        commandBuffer->begin();

        renderTargetBarrier.texture = swapChain->getCurrentTexture();
        // barriers are now handled by the render graph
        // commandBuffer->pipelineBarrier({renderTargetBarrier});
        depthBarrier.texture = swapChain->getCurrentDepthTexture();
        // commandBuffer->pipelineBarrier({depthBarrier});
        return commandBuffer;
    }

    void RenderHardwareInterface::endFrame() {
        CommandBuffer* commandBuffer = device->getCurrentCommandBuffer();
        presentBarrier.texture = renderTargetBarrier.texture;
        // commandBuffer->pipelineBarrier({presentBarrier});
        commandBuffer->end();

        device->submitCommandBuffers({commandBuffer}, swapChain.get());

        if (!swapChain->present()) {
            recreateSwapChain();
        }
    }

    std::unique_ptr<Shader> RenderHardwareInterface::createShader(const std::string& vertexSource, const std::string& fragmentSource) {
        return device->createShader(vertexSource, fragmentSource);
    }

    std::unique_ptr<Pipeline> RenderHardwareInterface::createPipeline(const Pipeline::CreateInfo& pipelineCreateInfo) {
        return device->createPipeline(pipelineCreateInfo);
    }

    std::unique_ptr<Buffer> RenderHardwareInterface::createBuffer(const Buffer::CreateInfo& bufferCreateInfo) {
        return device->createBuffer(bufferCreateInfo);
    }

    std::unique_ptr<Texture> RenderHardwareInterface::createTexture(const Texture::CreateInfo& textureCreateInfo) {
        return device->createTexture(textureCreateInfo);
    }

    std::unique_ptr<Sampler> RenderHardwareInterface::createSampler(const Sampler::CreateInfo& samplerCreateInfo) {
        return device->createSampler(samplerCreateInfo);
    }

    std::unique_ptr<ResourceLayout> RenderHardwareInterface::createResourceLayout(const std::vector<ResourceLayout::BindingCreateInfo>& bindings) {
        return device->createResourceLayout(bindings);
    }

    std::unique_ptr<ResourceSet> RenderHardwareInterface::createResourceSet(const ResourceLayout* resourceLayout) {
        return device->createResourceSet(resourceLayout);
    }

    std::unique_ptr<CommandBuffer> RenderHardwareInterface::beginSingleTimeCommands() {
        return device->beginSingleTimeCommands();
    }

    void RenderHardwareInterface::endSingleTimeCommands(CommandBuffer* commandBuffer) {
        device->endSingleTimeCommands(commandBuffer);
    }

    Pipeline* RenderHardwareInterface::getOrCreatePipeline(const Pipeline::CreateInfo& pipelineCreateInfo) {
        if (pipelineCache.find(pipelineCreateInfo) != pipelineCache.end()) {
            return pipelineCache[pipelineCreateInfo].get();
        }

        std::unique_ptr<Pipeline> newPipeline = createPipeline(pipelineCreateInfo);
        Pipeline* pNewPipeline = newPipeline.get();
        pipelineCache[pipelineCreateInfo] = std::move(newPipeline);

        return pNewPipeline;
    }

    void RenderHardwareInterface::recreateSwapChain() {
        uint32_t width = window->getFramebufferWidth();
        uint32_t height = window->getFramebufferHeight();

        device->waitIdle();
        swapChain.reset();
        swapChain = device->createSwapchain(Format::B8G8R8A8Unorm, width, height);
    }

    void RenderHardwareInterface::createDefaultSamplers() {
        Sampler::CreateInfo linearSamplerCreateInfo = {
            .adressMode = SamplerAddressMode::Repeat, .filterMode = SamplerFilterMode::Linear, .mipmapFilterMode = SamplerFilterMode::Linear};
        linearSampler = device->createSampler(linearSamplerCreateInfo);

        Sampler::CreateInfo nearestSamplerCreateInfo = {
            .adressMode = SamplerAddressMode::Repeat, .filterMode = SamplerFilterMode::Nearest, .mipmapFilterMode = SamplerFilterMode::Nearest};
        nearestSampler = device->createSampler(nearestSamplerCreateInfo);
    }
} // namespace Axiom
