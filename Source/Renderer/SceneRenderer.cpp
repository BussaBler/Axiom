#include "Renderer/SceneRenderer.h"

#include "Renderer/CommandBuffer.h"
#include "Renderer/ForwardRenderPipeline.h"
#include "Renderer/PSOManager.h"
#include "Renderer/RenderGraph.h"
#include "Renderer/RenderHardwareInterface.h"
#include "Renderer/RenderPipeline.h"

#include <memory>

namespace Axiom {
    SceneRenderer::SceneRenderer(RenderHardwareInterface& rhi, PSOManager& psoManager, AssetManager& assetManager) : rhi(rhi), assetManager(assetManager) {
        textureRegistry = std::make_unique<TextureRegistry>(rhi, assetManager);
        textureRegistry->init();
        textureRegistry->setFallbackTexture(assetManager.getDefaultTexture()->getTexture());
        renderGraph = std::make_unique<RenderGraph>(rhi);
        frp = std::make_unique<ForwardRenderPipeline>(rhi, assetManager, *textureRegistry, psoManager);
    }

    void SceneRenderer::beginFrame() const {
        renderGraph->begin();
        frp->beginFrame();
    }

    void SceneRenderer::execute(CommandBuffer* commandBuffer) const {
        renderGraph->execute(commandBuffer, {.frameIndex = rhi.getCurrentFrameIndex()});
    }

    void SceneRenderer::renderSceneView(const RenderContext& view) {
        frp->buildRenderGraph(*renderGraph, view);
    }
} // namespace Axiom
