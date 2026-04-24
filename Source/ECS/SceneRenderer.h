#pragma once
#include "Asset/ShaderAsset.h"
#include "Renderer/Renderer.h"
#include "Scene.h"

namespace Axiom {
    struct SceneRenderPassData {
        Scene* scene;
        CommandBuffer* commandBuffer;
        Texture* renderTarget;
        Texture* depthTarget;
        Math::Mat4 projection;
        Math::Mat4 view;
    };

    class SceneRenderer {
      public:
        SceneRenderer();
        ~SceneRenderer() = default;

        void geometryPass(const SceneRenderPassData& data);
        void lightingPass(const SceneRenderPassData& data);
        void postProcessPass(const SceneRenderPassData& data);
        void gizmoPass(const SceneRenderPassData& data, const Math::Vec3& gizmoPosition);

      private:
        void createGeometryPassResources();

        void createGizmoPassResources();

      private:
        // geometry pass data
        RenderPass geometryRenderPass;
        // 2d objects
        static const uint32_t MAX_SPRITE_INSTANCES = 1000;
        static const uint8_t MAX_TEXTURE_SLOTS = 16;
        std::shared_ptr<ShaderAsset> spriteShader = nullptr;
        std::unique_ptr<Pipeline> spritePipeline = nullptr;
        std::unique_ptr<Buffer> spriteVertexBuffer = nullptr;
        std::unique_ptr<Buffer> spriteIndexBuffer = nullptr;
        std::unique_ptr<Buffer> spriteInstanceBuffer = nullptr;
        std::vector<std::unique_ptr<ResourceLayout>> spriteResourceLayouts;
        std::vector<std::unique_ptr<ResourceSet>> spriteResourceSets;
        struct SpriteInstance {
            Math::Mat4 model;
            Math::Vec4 color;
            Math::Vec4 data;
        };
        struct SpriteVertex {
            Math::Vec2 position;
            Math::Vec2 uv;
        };
        // 3d objects
        static const uint32_t MAX_MESH_INSTANCES = 1000;
        std::shared_ptr<ShaderAsset> meshShader = nullptr;
        std::unique_ptr<Pipeline> meshPipeline = nullptr;
        std::unique_ptr<Buffer> meshInstanceBuffer = nullptr;
        std::vector<std::unique_ptr<ResourceLayout>> meshResourceLayouts;
        std::vector<std::unique_ptr<ResourceSet>> meshResourceSets;
        struct MeshInstance {
            Math::Mat4 model;
        };

        // gizmo pass data
        static const uint32_t MAX_GIZMO_LINES = 1000;
        std::shared_ptr<ShaderAsset> gizmoShader = nullptr;
        std::unique_ptr<Pipeline> gizmoPipeline = nullptr;
        RenderPass gizmoRenderPass;
        std::unique_ptr<Buffer> gizmoVertexBuffer = nullptr;
        struct GizmoVertex {
            Math::Vec4 position;
            Math::Vec4 color;
        };
    };
} // namespace Axiom