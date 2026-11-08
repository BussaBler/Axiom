#pragma once

#include "Asset/AssetManager.h"
#include "Renderer/CommandBuffer.h"
#include "Renderer/ForwardRenderPipeline.h"
#include "Renderer/PSOManager.h"
#include "Renderer/RenderGraph.h"
#include "Renderer/RenderHardwareInterface.h"
#include "Renderer/RenderPipeline.h"
#include "Renderer/TextureRegistry.h"

#include <memory>

namespace Axiom {
    class SceneRenderer {
      public:
        SceneRenderer(RenderHardwareInterface& rhi, PSOManager& psoManager, AssetManager& assetManager);
        ~SceneRenderer() = default;

        void beginFrame() const;
        void execute(CommandBuffer* commandBuffer) const;

        void renderSceneView(const RenderContext& view);

        RenderGraph& getRenderGraph() { return *renderGraph; }

      private:
        RenderHardwareInterface& rhi;
        AssetManager& assetManager;

        std::unique_ptr<TextureRegistry> textureRegistry;
        std::unique_ptr<RenderGraph> renderGraph;

        std::unique_ptr<ForwardRenderPipeline> frp;
    };
} // namespace Axiom
