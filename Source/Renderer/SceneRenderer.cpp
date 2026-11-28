#include "Renderer/SceneRenderer.h"

#include "Renderer/ForwardRenderPipeline.h"
#include "Renderer/PSOManager.h"
#include "Renderer/RenderGraph.h"
#include "Renderer/RenderHardwareInterface.h"
#include "Renderer/RenderPipeline.h"

#include <memory>

namespace Axiom {
    SceneRenderer::SceneRenderer(RenderHardwareInterface& rhi, PSOManager& psoManager, AssetManager& assetManager, TextureRegistry& textureRegistry)
        : rhi(rhi), assetManager(assetManager), textureRegistry(textureRegistry) {
        frp = std::make_unique<ForwardRenderPipeline>(rhi, assetManager, textureRegistry, psoManager);
    }

    void SceneRenderer::beginFrame() {
        views.clear();
    }

    void SceneRenderer::submitView(const RenderContext& view) {
        views.push_back(view);
    }

    void SceneRenderer::addToRenderGraph(RenderGraph& renderGraph) {
        frp->beginFrame();
        for (const RenderContext& view : views) {
            frp->buildRenderGraph(renderGraph, view);
        }
    }
} // namespace Axiom
