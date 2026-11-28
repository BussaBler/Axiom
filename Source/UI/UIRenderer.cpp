#include "axpch.h"

#include "UIRenderer.h"

#include "Asset/AssetManager.h"
#include "Core/Log.h"
#include "Core/Window.h"
#include "Math/AxMath.h"
#include "Math/Vec.h"
#include "Renderer/RenderHardwareInterface.h"
#include "Renderer/Texture.h"
#include "Renderer/TextureRegistry.h"
#include "UI/UIVertex.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <vector>

namespace Axiom {
    namespace {
        constexpr uint32_t INVALID_IMAGE_INDEX = std::numeric_limits<uint32_t>::max();
        constexpr uint32_t FALLBACK_TEXTURE_SLOT = 0;
    } // namespace

    UIRenderer::UIRenderer(RenderHardwareInterface& rhi, Window& window, AssetManager& assetManager, TextureRegistry& textureRegistry)
        : rhi(rhi), window(window), assetManager(assetManager), textureRegistry(textureRegistry) {
        createFontAtlas();

        for (auto& frame : frames) {
            frame.imageSlots = textureRegistry.reserveSlots(MAX_IMAGES_PER_FRAME);
            ensureCapacity(frame, INITIAL_CAPACITY_QUADS);
        }

        createPipeline();
        beginFrame();
    }

    void UIRenderer::beginFrame() {
        for (auto& list : drawLists) {
            list.vertices.clear();
            list.commands.clear();
            list.commands.push_back(DrawCommand{});
        }
        scissorRectStack = {};
        frameImages.clear();
    }

    void UIRenderer::addToRenderGraph(RenderGraph& renderGraph, Texture* renderTarget) {
        const uint32_t frameIndex = rhi.getCurrentFrameIndex();
        FrameResources& frame = frames[frameIndex];

        size_t totalVertices = 0;
        for (const auto& list : drawLists) {
            totalVertices += list.vertices.size();
        }
        if (totalVertices == 0) {
            return;
        }
        const uint32_t totalQuads = static_cast<uint32_t>(totalVertices / 4);

        ensureCapacity(frame, totalQuads);

        for (size_t i = 0; i < frameImages.size(); i++) {
            textureRegistry.setSlot(frame.imageSlots[i], frameImages[i]);
        }

        uploadScratch.clear();
        uploadScratch.reserve(totalVertices);
        for (const auto& list : drawLists) {
            for (UIVertex vertex : list.vertices) {
                if (vertex.primitive == static_cast<uint32_t>(UIPrimitive::Image)) {
                    vertex.textureIndex = vertex.textureIndex < frameImages.size() ? frame.imageSlots[vertex.textureIndex] : FALLBACK_TEXTURE_SLOT;
                }
                uploadScratch.push_back(vertex);
            }
        }
        frame.vertexBuffer->setData(uploadScratch.data(), uploadScratch.size() * sizeof(UIVertex));

        renderGraph.addPass<UIPassData>("UI Pass", [this, renderTarget, frameIndex](PassBuilder& builder, UIPassData& passData, Blackboard&) {
            passData.target = builder.importTexture("UI Target", renderTarget);
            builder.writeTexture(passData.target, TextureState::RenderTarget);

            for (Texture* image : frameImages) {
                builder.readTexture(builder.importTexture("UI Image", image), TextureState::ShaderResource);
            }

            return [this, frameIndex](const UIPassData& passData, const PassResources& resources, CommandBuffer* commandBuffer, const Blackboard&,
                                      const FrameContext&) {
                const Texture* target = resources.getTexture(passData.target);
                if (!target) {
                    return;
                }
                const FrameResources& frame = frames[frameIndex];

                const Math::iVec2 targetSize = target->getSize();
                const Math::Mat4 projection =
                    Math::Mat4::orthographic(0.0f, static_cast<float>(window.getWidth()), 0.0f, static_cast<float>(window.getHeight()), -1.0f, 1.0f);

                RenderPass renderPass{};
                renderPass.colorAttachments[0] = RenderAttachment{.texture = target, .loadOp = LoadOp::Load, .storeOp = StoreOp::Store};
                renderPass.colorAttachmentCount = 1;
                renderPass.width = static_cast<uint32_t>(targetSize.x());
                renderPass.height = static_cast<uint32_t>(targetSize.y());

                commandBuffer->beginRendering(renderPass);
                commandBuffer->bindPipeline(pipeline.get());
                commandBuffer->setViewport(0.0f, 0.0f, static_cast<float>(targetSize.x()), static_cast<float>(targetSize.y()));
                commandBuffer->bindPushConstants(&projection, sizeof(projection));
                commandBuffer->bindResources({textureRegistry.getResourceSet()}, 0);
                commandBuffer->bindVertexBuffers({frame.vertexBuffer.get()});
                commandBuffer->bindIndexBuffer(frame.indexBuffer.get());

                const Math::Rect fullTarget(Math::Vec2(0.0f), Math::Vec2(static_cast<float>(targetSize.x()), static_cast<float>(targetSize.y())));

                uint32_t layerFirstQuad = 0;
                for (const auto& list : drawLists) {
                    for (const auto& command : list.commands) {
                        if (command.quadCount == 0) {
                            continue;
                        }

                        const Math::Rect scissor = command.hasScissor ? command.scissor.getIntersection(fullTarget) : fullTarget;
                        if (scissor.width() <= 0.0f || scissor.height() <= 0.0f) {
                            continue;
                        }
                        commandBuffer->setScissor(static_cast<int32_t>(scissor.x()), static_cast<int32_t>(scissor.y()), static_cast<uint32_t>(scissor.width()),
                                                  static_cast<uint32_t>(scissor.height()));

                        const int32_t vertexOffset = static_cast<int32_t>((layerFirstQuad + command.firstQuad) * 4);
                        commandBuffer->drawIndexed(command.quadCount * 6, 1, 0, vertexOffset, 0);
                    }
                    layerFirstQuad += static_cast<uint32_t>(list.vertices.size() / 4);
                }

                commandBuffer->endRendering();
            };
        });
    }

    bool UIRenderer::isValidLayer(uint8_t layer, const char* operation) const {
        if (layer < MAX_RENDER_LAYERS) {
            return true;
        }
        AX_CORE_LOG_ERROR("UIRenderer: {} on invalid layer {} (highest is {})", operation, layer, MAX_RENDER_LAYERS - 1);
        return false;
    }

    void UIRenderer::startCommand(uint8_t layer, std::optional<Math::Rect> scissor) {
        DrawList& list = drawLists[layer];
        const uint32_t nextQuad = static_cast<uint32_t>(list.vertices.size() / 4);

        DrawCommand command{};
        command.firstQuad = nextQuad;
        command.hasScissor = scissor.has_value();
        if (scissor) {
            command.scissor = *scissor;
        }

        if (!list.commands.empty() && list.commands.back().quadCount == 0) {
            list.commands.back() = command;
        } else {
            list.commands.push_back(command);
        }
    }

    void UIRenderer::pushQuad(uint8_t layer, const UIVertex& topLeft, const UIVertex& topRight, const UIVertex& bottomRight, const UIVertex& bottomLeft) {
        DrawList& list = drawLists[layer];
        list.vertices.push_back(topLeft);
        list.vertices.push_back(topRight);
        list.vertices.push_back(bottomRight);
        list.vertices.push_back(bottomLeft);
        list.commands.back().quadCount += 1;
    }

    void UIRenderer::addBasicQuad(const Math::Vec2& pos, const Math::Vec2& size, const Color& color, const Math::Vec4& radii, uint8_t layer) {
        if (!isValidLayer(layer, "addBasicQuad")) {
            return;
        }
        const Math::Vec4 params(size.x(), size.y(), 0.0f, 0.0f);
        pushQuad(layer, UIVertex(pos, Math::Vec2(0.0f, 0.0f), color, params, radii, 0, UIPrimitive::Rect),
                 UIVertex(pos + Math::Vec2(size.x(), 0.0f), Math::Vec2(1.0f, 0.0f), color, params, radii, 0, UIPrimitive::Rect),
                 UIVertex(pos + size, Math::Vec2(1.0f, 1.0f), color, params, radii, 0, UIPrimitive::Rect),
                 UIVertex(pos + Math::Vec2(0.0f, size.y()), Math::Vec2(0.0f, 1.0f), color, params, radii, 0, UIPrimitive::Rect));
    }

    void UIRenderer::addDebugRect(const Math::Vec2& pos, const Math::Vec2& size, const Color& color, uint8_t layer) {
        const float thickness = 1.0f;
        addBasicQuad(pos, Math::Vec2(size.x(), thickness), color, Math::Vec4::zero(), layer);
        addBasicQuad(pos + Math::Vec2(0.0f, size.y() - thickness), Math::Vec2(size.x(), thickness), color, Math::Vec4::zero(), layer);
        addBasicQuad(pos, Math::Vec2(thickness, size.y()), color, Math::Vec4::zero(), layer);
        addBasicQuad(pos + Math::Vec2(size.x() - thickness, 0.0f), Math::Vec2(thickness, size.y()), color, Math::Vec4::zero(), layer);
    }

    void UIRenderer::addFontQuad(const Math::Vec2& pos, const Math::Vec2& size, const Math::Vec2& uv0, const Math::Vec2& uv1, const Color& color,
                                 uint8_t layer) {
        if (!isValidLayer(layer, "addFontQuad")) {
            return;
        }
        const Math::Vec4 zero = Math::Vec4::zero();
        pushQuad(layer, UIVertex(pos, uv0, color, zero, zero, fontAtlasSlot, UIPrimitive::Glyph),
                 UIVertex(pos + Math::Vec2(size.x(), 0.0f), Math::Vec2(uv1.x(), uv0.y()), color, zero, zero, fontAtlasSlot, UIPrimitive::Glyph),
                 UIVertex(pos + size, uv1, color, zero, zero, fontAtlasSlot, UIPrimitive::Glyph),
                 UIVertex(pos + Math::Vec2(0.0f, size.y()), Math::Vec2(uv0.x(), uv1.y()), color, zero, zero, fontAtlasSlot, UIPrimitive::Glyph));
    }

    void UIRenderer::addImageQuad(const Math::Vec2& pos, const Math::Vec2& size, Texture* texture, uint8_t layer) {
        if (!isValidLayer(layer, "addImageQuad") || !texture) {
            return;
        }

        uint32_t localIndex = INVALID_IMAGE_INDEX;
        auto it = std::find(frameImages.begin(), frameImages.end(), texture);
        if (it != frameImages.end()) {
            localIndex = static_cast<uint32_t>(it - frameImages.begin());
        } else if (frameImages.size() < MAX_IMAGES_PER_FRAME) {
            localIndex = static_cast<uint32_t>(frameImages.size());
            frameImages.push_back(texture);
        } else {
            AX_CORE_LOG_ERROR_ONCE("UIRenderer: more than {} different images in one frame, extra images use the fallback texture", MAX_IMAGES_PER_FRAME);
        }

        const Color white = Color::white();
        const Math::Vec4 zero = Math::Vec4::zero();
        pushQuad(layer, UIVertex(pos, Math::Vec2(0.0f, 0.0f), white, zero, zero, localIndex, UIPrimitive::Image),
                 UIVertex(pos + Math::Vec2(size.x(), 0.0f), Math::Vec2(1.0f, 0.0f), white, zero, zero, localIndex, UIPrimitive::Image),
                 UIVertex(pos + size, Math::Vec2(1.0f, 1.0f), white, zero, zero, localIndex, UIPrimitive::Image),
                 UIVertex(pos + Math::Vec2(0.0f, size.y()), Math::Vec2(0.0f, 1.0f), white, zero, zero, localIndex, UIPrimitive::Image));
    }

    void UIRenderer::addText(const std::string& text, const Math::Vec2& pos, float fontSize, const Color& color, uint8_t layer) {
        const auto& asciiAtlas = openSansFont.getAsciiAtlas();

        const float pixelSize = fontSize;
        const float fontScale = pixelSize / asciiAtlas.unitsPerEm;
        const float lineSpacing = pixelSize * 1.2f;

        const float dpiScale = std::max(1.0f, static_cast<float>(window.getWindowDPI()) / 96.0f);
        const auto snap = [dpiScale](float v) { return std::round(v * dpiScale) / dpiScale; };

        float cursorX = snap(pos.x());
        const float ascender = asciiAtlas.ascender != 0 ? asciiAtlas.ascender : asciiAtlas.unitsPerEm * 0.8f;
        float cursorY = snap(pos.y() + (ascender * fontScale));

        for (char c : text) {
            if (c == '\n') {
                cursorX = pos.x();
                cursorY += lineSpacing;
                continue;
            }

            auto glyphIt = asciiAtlas.glyphs.find(static_cast<uint32_t>(c));
            if (glyphIt == asciiAtlas.glyphs.end()) {
                continue;
            }
            const GlyphMetrics& metrics = glyphIt->second;

            if (c != ' ' && c != '\t') {
                const float minX = cursorX + (metrics.quadMin.x() * fontScale);
                const float minY = cursorY - (metrics.quadMax.y() * fontScale);
                const float maxX = cursorX + (metrics.quadMax.x() * fontScale);
                const float maxY = cursorY - (metrics.quadMin.y() * fontScale);
                addFontQuad(Math::Vec2(minX, minY), Math::Vec2(maxX - minX, maxY - minY), metrics.uv0, metrics.uv1, color, layer);
            }

            cursorX += (metrics.advance * fontScale);
        }
    }

    float UIRenderer::calculateTextWidth(const std::string& text, float fontSize) {
        const auto& asciiAtlas = openSansFont.getAsciiAtlas();
        const float pixelSize = fontSize;
        const float fontScale = pixelSize / asciiAtlas.unitsPerEm;

        float maxWidth = 0.0f;
        float currentLineWidth = 0.0f;
        for (char c : text) {
            if (c == '\n') {
                maxWidth = std::max(maxWidth, currentLineWidth);
                currentLineWidth = 0.0f;
                continue;
            }
            auto glyphIt = asciiAtlas.glyphs.find(static_cast<uint32_t>(c));
            if (glyphIt != asciiAtlas.glyphs.end()) {
                currentLineWidth += glyphIt->second.advance * fontScale;
            }
        }
        return std::max(maxWidth, currentLineWidth);
    }

    float UIRenderer::getCenteredTextOffset(float boxHeight, float fontSize) {
        const auto& asciiAtlas = openSansFont.getAsciiAtlas();
        const float fontScale = fontSize / asciiAtlas.unitsPerEm;
        const float ascender = (asciiAtlas.ascender != 0 ? asciiAtlas.ascender : asciiAtlas.unitsPerEm * 0.8f) * fontScale;

        // TODO: make this work with other fonts
        constexpr float CAP_HEIGHT_RATIO = 0.714f;
        const float capHeight = fontSize * CAP_HEIGHT_RATIO;

        return (boxHeight + capHeight) * 0.5f - ascender;
    }

    float UIRenderer::calculateTextHeight(float fontSize) {
        const auto& asciiAtlas = openSansFont.getAsciiAtlas();
        const float pixelSize = fontSize;
        const float fontScale = pixelSize / asciiAtlas.unitsPerEm;

        const float ascender = asciiAtlas.ascender != 0 ? asciiAtlas.ascender : asciiAtlas.unitsPerEm * 0.8f;
        const float descender = asciiAtlas.descender != 0 ? asciiAtlas.descender : -asciiAtlas.unitsPerEm * 0.2f;
        return (ascender - descender) * fontScale;
    }

    void UIRenderer::pushScissorRect(const Math::Rect& rect, uint8_t layer) {
        if (!isValidLayer(layer, "pushScissorRect")) {
            return;
        }
        const float dpiScale = static_cast<float>(window.getWindowDPI()) / 96.0f;
        Math::Rect scaled(rect.getPos() * dpiScale, rect.getSize() * dpiScale);

        if (!scissorRectStack.empty()) {
            scaled = scaled.getIntersection(scissorRectStack.top());
        }

        scissorRectStack.push(scaled);
        startCommand(layer, scaled);
    }

    void UIRenderer::popScissorRect(uint8_t layer) {
        if (!isValidLayer(layer, "popScissorRect")) {
            return;
        }
        if (!scissorRectStack.empty()) {
            scissorRectStack.pop();
        }
        startCommand(layer, scissorRectStack.empty() ? std::nullopt : std::optional<Math::Rect>(scissorRectStack.top()));
    }

    void UIRenderer::ensureCapacity(FrameResources& frame, uint32_t quadCount) {
        if (frame.vertexBuffer && quadCount <= frame.capacityQuads) {
            return;
        }

        uint32_t newCapacity = std::max(frame.capacityQuads, INITIAL_CAPACITY_QUADS);
        while (newCapacity < quadCount) {
            newCapacity *= 2;
        }
        if (frame.vertexBuffer) {
            AX_CORE_LOG_INFO("UIRenderer: growing UI buffers from {} to {} quads", frame.capacityQuads, newCapacity);
        }

        frame.vertexBuffer =
            rhi.createBuffer({.size = sizeof(UIVertex) * 4ull * newCapacity, .usage = BufferUsage::Vertex, .memoryUsage = MemoryUsage::GPUandCPU});
        frame.indexBuffer =
            rhi.createBuffer({.size = sizeof(uint32_t) * 6ull * newCapacity, .usage = BufferUsage::Index, .memoryUsage = MemoryUsage::GPUandCPU});

        std::vector<uint32_t> indices(static_cast<size_t>(newCapacity) * 6);
        for (uint32_t quad = 0; quad < newCapacity; quad++) {
            const uint32_t v = quad * 4;
            const size_t i = static_cast<size_t>(quad) * 6;
            indices[i + 0] = v + 0;
            indices[i + 1] = v + 1;
            indices[i + 2] = v + 2;
            indices[i + 3] = v + 0;
            indices[i + 4] = v + 2;
            indices[i + 5] = v + 3;
        }
        frame.indexBuffer->setData(indices.data(), indices.size() * sizeof(uint32_t));
        frame.capacityQuads = newCapacity;
    }

    void UIRenderer::createFontAtlas() {
        const auto& asciiAtlas = openSansFont.getAsciiAtlas();
        Texture::CreateInfo fontAtlasCreateInfo = {.width = asciiAtlas.width,
                                                   .height = asciiAtlas.height,
                                                   .mipLevels = 1,
                                                   .arrayLayers = 1,
                                                   .format = Format::R8G8B8A8Unorm,
                                                   .usage = TextureUsage::Sampled | TextureUsage::TransferDst,
                                                   .aspect = TextureAspect::Color,
                                                   .memoryUsage = MemoryUsage::GPUOnly};
        fontAtlasTexture = rhi.createTexture(fontAtlasCreateInfo);

        std::unique_ptr<Buffer> stagingBuffer = rhi.createBuffer(
            {.size = asciiAtlas.width * asciiAtlas.height * asciiAtlas.channels, .usage = BufferUsage::TransferSrc, .memoryUsage = MemoryUsage::GPUandCPU});
        stagingBuffer->setData<uint8_t>(asciiAtlas.pixels);

        auto commandBuffer = rhi.beginSingleTimeCommands();
        commandBuffer->copyBufferToTexture(stagingBuffer.get(), fontAtlasTexture.get(), asciiAtlas.width, asciiAtlas.height);
        rhi.endSingleTimeCommands(commandBuffer.get());

        fontAtlasSlot = textureRegistry.reserveSlots(1).front();
        textureRegistry.setSlot(fontAtlasSlot, fontAtlasTexture.get());
    }

    void UIRenderer::createPipeline() {
        shader = assetManager.getBuiltInShader("UI");

        std::vector<VertexBindingDescription> vertexBindings = {{.binding = 0, .stride = sizeof(UIVertex), .inputRate = VertexInputRate::Vertex}};
        std::vector<VertexAttributeDescription> vertexAttributes = {
            {.location = 0, .binding = 0, .format = Format::R32G32Sfloat, .offset = offsetof(UIVertex, position)},
            {.location = 1, .binding = 0, .format = Format::R32G32Sfloat, .offset = offsetof(UIVertex, uv)},
            {.location = 2, .binding = 0, .format = Format::R32G32B32A32Sfloat, .offset = offsetof(UIVertex, color)},
            {.location = 3, .binding = 0, .format = Format::R32G32B32A32Sfloat, .offset = offsetof(UIVertex, params)},
            {.location = 4, .binding = 0, .format = Format::R32G32B32A32Sfloat, .offset = offsetof(UIVertex, radii)},
            {.location = 5, .binding = 0, .format = Format::R32UInt, .offset = offsetof(UIVertex, textureIndex)},
            {.location = 6, .binding = 0, .format = Format::R32UInt, .offset = offsetof(UIVertex, primitive)},
        };

        Pipeline::CreateInfo pipelineCreateInfo = {.shader = shader->getShader(),
                                                   .vertexBindings = vertexBindings,
                                                   .vertexAttributes = vertexAttributes,
                                                   .topology = PrimitiveTopology::TriangleList,
                                                   .polygonMode = PolygonMode::Fill,
                                                   .cullMode = CullMode::None,
                                                   .frontFaceClockwise = true,
                                                   .enableBlending = true,
                                                   .enableDepthTest = false,
                                                   .enableDepthWrite = false,
                                                   .colorAttachmentFormats = {rhi.getRenderTargetFormat()},
                                                   .depthAttachmentFormat = Format::Undefined,
                                                   .resourceLayouts = {textureRegistry.getResourceLayout()}};
        pipeline = rhi.createPipeline(pipelineCreateInfo);
    }
} // namespace Axiom
