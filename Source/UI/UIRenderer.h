#pragma once

#include "Asset/AssetManager.h"
#include "Asset/ShaderAsset.h"
#include "Core/Window.h"
#include "Font.h"
#include "Math/AxMath.h"
#include "Math/Color.h"
#include "Renderer/Buffer.h"
#include "Renderer/Pipeline.h"
#include "Renderer/RenderGraph.h"
#include "Renderer/RenderHardwareInterface.h"
#include "UIVertex.h"

#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <stack>
#include <string>
#include <vector>

namespace Axiom {
    class TextureRegistry;

    class UIRenderer {
      public:
        UIRenderer(RenderHardwareInterface& rhi, Window& window, AssetManager& assetManager, TextureRegistry& textureRegistry);
        ~UIRenderer() = default;

        UIRenderer(const UIRenderer&) = delete;
        UIRenderer& operator=(const UIRenderer&) = delete;

        void beginFrame();
        void addToRenderGraph(RenderGraph& renderGraph, Texture* renderTarget);

        void addBasicQuad(const Math::Vec2& pos, const Math::Vec2& size, const Color& color, const Math::Vec4& radii = Math::Vec4::zero(), uint8_t layer = 0);
        void addDebugRect(const Math::Vec2& pos, const Math::Vec2& size, const Color& color, uint8_t layer = 0);
        void addFontQuad(const Math::Vec2& pos, const Math::Vec2& size, const Math::Vec2& uv0, const Math::Vec2& uv1, const Color& color, uint8_t layer = 0);
        void addText(const std::string& text, const Math::Vec2& pos, float fontSize, const Color& color, uint8_t layer = 0);
        void addImageQuad(const Math::Vec2& pos, const Math::Vec2& size, Texture* texture, uint8_t layer = 0);

        float calculateTextWidth(const std::string& text, float fontSize);
        float calculateTextHeight(float fontSize);
        float getCenteredTextOffset(float boxHeight, float fontSize);

        void pushScissorRect(const Math::Rect& rect, uint8_t layer = 0);
        void popScissorRect(uint8_t layer = 0);

        inline Font& getFont() { return openSansFont; }

      public:
        static constexpr uint8_t MAX_RENDER_LAYERS = 4;
        static constexpr uint32_t MAX_IMAGES_PER_FRAME = 32;

      private:
        struct DrawCommand {
            uint32_t firstQuad = 0;
            uint32_t quadCount = 0;
            Math::Rect scissor;
            bool hasScissor = false;
        };

        struct DrawList {
            std::vector<UIVertex> vertices;
            std::vector<DrawCommand> commands;
        };

        struct FrameResources {
            std::unique_ptr<Buffer> vertexBuffer;
            std::unique_ptr<Buffer> indexBuffer;
            uint32_t capacityQuads = 0;
            std::vector<uint32_t> imageSlots;
        };

        struct UIPassData {
            RGTextureHandle target;
        };

      private:
        void createPipeline();
        void createFontAtlas();
        void ensureCapacity(FrameResources& frame, uint32_t quadCount);
        void pushQuad(uint8_t layer, const UIVertex& topLeft, const UIVertex& topRight, const UIVertex& bottomRight, const UIVertex& bottomLeft);
        void startCommand(uint8_t layer, std::optional<Math::Rect> scissor);
        bool isValidLayer(uint8_t layer, const char* operation) const;

      private:
        RenderHardwareInterface& rhi;
        Window& window;
        AssetManager& assetManager;
        TextureRegistry& textureRegistry;

        Font openSansFont{"axiom://Packages/Fonts/OpenSans-SemiBold.ttf"};
        std::shared_ptr<Texture> fontAtlasTexture = nullptr;
        uint32_t fontAtlasSlot = 0;

        std::shared_ptr<ShaderAsset> shader = nullptr;
        std::unique_ptr<Pipeline> pipeline = nullptr;

        std::array<DrawList, MAX_RENDER_LAYERS> drawLists;
        std::stack<Math::Rect> scissorRectStack;

        std::vector<Texture*> frameImages;

        std::array<FrameResources, RenderHardwareInterface::MAX_FRAMES_IN_FLIGHT> frames;
        std::vector<UIVertex> uploadScratch;

        static constexpr uint32_t INITIAL_CAPACITY_QUADS = 4096;
    };
} // namespace Axiom
