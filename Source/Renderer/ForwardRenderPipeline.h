#pragma once
#include "Asset/MaterialAsset.h"
#include "Asset/MeshAsset.h"
#include "Math/Color.h"
#include "Math/Mat.h"
#include "Math/Vec.h"
#include "Renderer/Buffer.h"
#include "Renderer/CommandBuffer.h"
#include "Renderer/Pipeline.h"
#include "Renderer/RenderFeature.h"
#include "Renderer/RenderGraph.h"
#include "Renderer/RenderPass.h"
#include "Renderer/RenderPipeline.h"
#include "Renderer/ResourceLayout.h"
#include "Renderer/ResourceSet.h"
#include "Renderer/Texture.h"
#include "Scene/Scene.h"

#include <cstdint>
#include <memory>
#include <utility>
#include <vector>

namespace Axiom {
    struct GlobalResourceData {
        Scene* scene;
        RGTextureHandle colorBuffer;
        RGTextureHandle depthBuffer;
        uint32_t viewportIndex = 0;
        Math::Vec3 gizmoPosition = Math::Vec3::zero();
    };

    struct GizmosPassData {
        Scene* scene;
        RGTextureHandle renderTarget;
        RGTextureHandle depthTarget;
        uint8_t viewportIndex = 0;
        Math::Vec3 gizmosPosition = Math::Vec3::zero();
    };

    class ForwardRenderPipeline : public RenderPipeline {
      public:
        ForwardRenderPipeline();
        ~ForwardRenderPipeline();

        void beginFrame() override;
        void buildRenderGraph(RenderGraph& renderGraph, const RenderContext& context) override;

        inline void addRenderFeature(std::unique_ptr<RenderFeature> renderFeature) { renderFeatures.push_back(std::move(renderFeature)); }

      private:
        void createGlobalData();
        void createOpaquePassData();
        void createSpritePassData();
        void createSkyboxPassData();
        void createWorldGridPassData();
        void createGizmosPassData();

        void opaquePass(Scene* scene, const Texture* renderTarget, const Texture* depthTarget, CommandBuffer* commandBuffer, uint32_t viewportIndex);
        void spritePass(Scene* scene, const Texture* renderTarget, const Texture* depthtarget, CommandBuffer* commandBuffer, uint32_t viewportIndex);
        void skyboxPass(Scene* scene, const Texture* renderTarget, const Texture* depthTarget, CommandBuffer* commandBuffer);
        void transparentPass(Scene* scene, const Texture* renderTarget, const Texture* depthTarget, CommandBuffer* commandBuffer);
        void shadowPass(Scene* scene, const Texture* renderTarget, const Texture* depthTarget, CommandBuffer* commandBuffer);
        void worldGridPass(Scene* scene, const Texture* renderTarget, const Texture* depthTarget, CommandBuffer* commandBuffer);
        void gizmosPass(Scene* scene, const Texture* renderTarget, const Texture* depthTarget, CommandBuffer* commandBuffer, const Math::Vec3& gizmoPosition);

        void injectRenderFeatures(RenderInjectionPoint injectionPoint, RenderGraph& renderGraph, const RenderContext& context);

      private:
        struct GlobalData {
            Math::Mat4 viewMatrix = Math::Mat4::identity();
            Math::Mat4 projectionMatrix = Math::Mat4::identity();
            alignas(16) Math::Vec3 cameraPosition = Math::Vec3::zero();

            Color ambientColor = Color::white();
            alignas(16) Math::Vec3 mainLightDirection = Math::Vec3(0.0f, 1.0f, 0.0f);
            Color mainLightColor = Color::white();
        };

        struct DrawCommand {
            Math::Mat4 model = Math::Mat4::identity();
            std::shared_ptr<MeshAsset> meshAsset = nullptr;
            std::shared_ptr<MaterialAsset> materialAsset = nullptr;

            bool operator<(const DrawCommand& other) const {
                if (materialAsset->getShader() != other.materialAsset->getShader()) {
                    return materialAsset->getShader() < other.materialAsset->getShader();
                }
                return materialAsset->getHandle() < other.materialAsset->getHandle();
            }
        };

      private:
        static constexpr uint32_t GLOBAL_DATA_ALIGNMENT = 256;
        static constexpr uint32_t INSTANCE_DATA_ALIGNMENT = 256;
        static constexpr uint8_t MAX_VIEWPORTS = 10;
        static constexpr uint32_t MAX_INSTANCES_PER_FRAME = 10000;
        static constexpr uint32_t MAX_SPRITE_INSTANCES_PER_FRAME = 1000;
        uint32_t globalDataAllocations = 0;
        uint32_t totalInstanceCount = 0;

        GlobalData globalData;
        std::unique_ptr<ResourceLayout> globalDataResourceLayout;
        std::unique_ptr<ResourceSet> globalDataResourceSet;
        std::unique_ptr<Buffer> globalDataBuffer;

        RenderPass opaqueRenderPass;
        std::unique_ptr<ResourceLayout> instanceResourceLayout;
        std::unique_ptr<ResourceSet> instanceResourceSet;
        std::unique_ptr<Buffer> instanceSSBO;

        RenderPass spriteRenderPass;
        std::unique_ptr<Buffer> spriteVertexBuffer;
        std::unique_ptr<Buffer> spriteIndexBuffer;
        std::unique_ptr<ResourceLayout> spriteInstanceResourceLayout;
        std::unique_ptr<ResourceSet> spriteInstanceResourceSet;
        std::unique_ptr<Buffer> spriteInstanceSSBO;
        std::unique_ptr<Pipeline> spritePipeline;

        RenderPass skyboxRenderPass;
        std::unique_ptr<Buffer> skyboxVertexBuffer;
        std::unique_ptr<Buffer> skyboxIndexBuffer;
        std::unique_ptr<Pipeline> skyboxPipeline;

        RenderPass worldGridRenderPass;
        std::unique_ptr<Pipeline> worldGridPipeline;

        RenderPass gizmosRenderPass;
        UUID gizmosMeshHandle;
        std::unique_ptr<Pipeline> gizmosPipeline;

        std::vector<std::unique_ptr<RenderFeature>> renderFeatures;
    };
} // namespace Axiom
