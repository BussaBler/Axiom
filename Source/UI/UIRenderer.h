#pragma once
#include "Asset/AssetManager.h"
#include "Font.h"
#include "Math/Color.h"
#include "Renderer/Renderer.h"
#include "UIVertex.h"
#include "axpch.h"

namespace Axiom {
    class UIRenderer {
      public:
        UIRenderer();
        ~UIRenderer() = default;

        void clearRenderData();

        void addBasicQuad(const Math::Vec2& pos, const Math::Vec2& size, const Color& color, const Math::Vec4& radii = Math::Vec4::zero());
        void addDebugRect(const Math::Vec2& pos, const Math::Vec2& size, const Color& color);
        void addFontQuad(const Math::Vec2& pos, const Math::Vec2& size, const Math::Vec2& uv0, const Math::Vec2& uv1, const Color& color);
        void addImageQuad(const Math::Vec2& pos, const Math::Vec2& size, Texture* texture);

        void drawUIElements(CommandBuffer* commandBuffer, Texture* targetTexture);

        inline Font& getFont() { return openSansFont; }

      private:
        void createBasicRenderObjects();
        void createFontRenderObjects();
        void createImageRenderObjects();

        ResourceSet* getResourceSetForTexture(Texture* texture);

      private:
        static const uint32_t MAX_BASIC_QUADS = 1000;
        std::shared_ptr<ShaderAsset> basicShader = nullptr;
        std::unique_ptr<Pipeline> basicPipeline = nullptr;
        RenderPass basicRenderPass;
        std::unique_ptr<Buffer> basicVertexBuffer = nullptr;
        std::unique_ptr<Buffer> basicIndexBuffer = nullptr;
        std::vector<UIVertex> basicVertices;

        static const uint32_t MAX_FONT_QUADS = 1000;
        Font openSansFont{"Assets/Fonts/OpenSans-SemiBold.ttf"};
        std::shared_ptr<Texture> fontAtlasTexture = nullptr;
        std::shared_ptr<ShaderAsset> fontShader = nullptr;
        std::unique_ptr<Pipeline> fontPipeline = nullptr;
        RenderPass fontRenderPass;
        std::unique_ptr<Buffer> fontVertexBuffer = nullptr;
        std::unique_ptr<Buffer> fontIndexBuffer = nullptr;
        std::unique_ptr<ResourceLayout> fontResourceLayout = nullptr;
        std::unique_ptr<ResourceSet> fontResourceSet = nullptr;
        std::vector<UIVertex> fontVertices;

        struct ImageDrawCommand {
            Texture* texture;
            Math::Vec2 pos;
            Math::Vec2 size;
        };
        std::vector<ImageDrawCommand> imageDrawCommands;
        static const uint32_t MAX_IMAGE_QUADS = 100;
        std::shared_ptr<ShaderAsset> imageShader = nullptr;
        std::unique_ptr<Pipeline> imagePipeline = nullptr;
        RenderPass imageRenderPass;
        std::unique_ptr<Buffer> imageVertexBuffer = nullptr;
        std::unique_ptr<Buffer> imageIndexBuffer = nullptr;
        std::unique_ptr<ResourceLayout> imageResourceLayout = nullptr;
        std::unordered_map<Texture*, std::unique_ptr<ResourceSet>> imageResourceSets;
    };
}; // namespace Axiom