#pragma once

#include "Asset/AssetManager.h"
#include "Renderer/ForwardRenderPipeline.h"
#include "Renderer/PSOManager.h"
#include "Renderer/RenderGraph.h"
#include "Renderer/RenderHardwareInterface.h"
#include "Renderer/RenderPipeline.h"
#include "Renderer/TextureRegistry.h"

#include <memory>
#include <vector>

namespace Axiom {
    class SceneRenderer {
      public:
        SceneRenderer(RenderHardwareInterface& rhi, PSOManager& psoManager, AssetManager& assetManager, TextureRegistry& textureRegistry);
        ~SceneRenderer() = default;

        void beginFrame();
        void submitView(const RenderContext& view);
        void addToRenderGraph(RenderGraph& renderGraph);

      private:
        RenderHardwareInterface& rhi;
        AssetManager& assetManager;
        TextureRegistry& textureRegistry;

        std::unique_ptr<ForwardRenderPipeline> frp;

        std::vector<RenderContext> views;
    };
} // namespace Axiom
