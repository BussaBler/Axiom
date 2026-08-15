#pragma once

#include "Renderer/RenderGraph.h"
#include "Renderer/RenderPipeline.h"
namespace Axiom {
    enum class RenderInjectionPoint { BeforeOpaque, AfterOpaque, BeforeSkybox, AfterSkybox, AfterPostProcessing };

    class RenderFeature {
      public:
        virtual ~RenderFeature() = default;

        virtual void init() {}
        virtual RenderInjectionPoint getInjectionPoint() const = 0;
        virtual void injectPasses(RenderGraph& renderGraph, const RenderContext& renderView) = 0;
    };
} // namespace Axiom
