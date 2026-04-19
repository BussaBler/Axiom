#pragma once
#include "Asset/AssetManager.h"
#include "Component.h"
#include "Renderer/Renderer.h"
#include "SystemManager.h"

namespace Axiom {
    struct SpriteVertex {
        Math::Vec2 pos;
        Math::Vec2 uv;
    };

    struct SpriteInstance {
        Math::Mat4 model;
        Math::Vec4 color;
        Math::Vec4 data;
    };

    class RenderSystem : public System {
      public:
        RenderSystem();
        ~RenderSystem() override = default;

        void onUpdate(Registry* registry, float deltaTime) override;
        void onRender(Registry* registry, CommandBuffer* commandBuffer, Texture* renderTarget);

      private:
        void createSpriteRenderObjects();

        void flushAndDraw(CommandBuffer* commandBuffer, Texture* renderTarget, Math::Mat4 projection, std::vector<SpriteInstance>& instances,
                          std::vector<Texture*>& textureSlots);

      private:
        // 2D objects
        static const uint32_t MAX_SPRITE_QUADS = 1000;
        static const uint8_t MAX_TEXTURE_SLOTS = 16;
        std::shared_ptr<ShaderAsset> spriteShader = nullptr;
        std::unique_ptr<Pipeline> spritePipeline = nullptr;
        RenderPass spriteRenderPass;
        std::unique_ptr<Buffer> spriteVertexBuffer = nullptr;
        std::unique_ptr<Buffer> spriteIndexBuffer = nullptr;
        std::unique_ptr<Buffer> spriteInstanceBuffer = nullptr;
        std::vector<std::unique_ptr<ResourceLayout>> spriteResourceLayouts;
        std::vector<std::unique_ptr<ResourceSet>> spriteResourceSets;
    };
} // namespace Axiom