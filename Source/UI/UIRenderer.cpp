#include "axpch.h"

#include "UIRenderer.h"

#include "Asset/AssetManager.h"
#include "Core/Log.h"
#include "Core/TaskManager.h"
#include "Core/Window.h"
#include "Renderer/RenderHardwareInterface.h"
#include "Renderer/ResourceLayout.h"
#include "Renderer/Sampler.h"
#include "Renderer/Texture.h"

namespace Axiom {
    UIRenderer::UIRenderer(RenderHardwareInterface& rhi, Window& window, AssetManager& assetManager, TaskManager& taskManager)
        : rhi(rhi), window(window), assetManager(assetManager), taskManager(taskManager) {
        auto& asciiAtlas = openSansFont.getAsciiAtlas();
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
        createBasicRenderObjects();
        createFontRenderObjects();
        createImageRenderObjects();
    }

    void UIRenderer::beginFrame() {
        for (auto& layer : renderLayers) {
            layer.basicRenderBatches.clear();
            layer.fontRenderBatches.clear();
            layer.basicVertices.clear();
            layer.fontVertices.clear();
            layer.imageDrawCommands.clear();
        }
    }

    void UIRenderer::addBasicQuad(const Math::Vec2& pos, const Math::Vec2& size, const Color& color, const Math::Vec4& radii, uint8_t layer) {
        if (renderLayers[layer].basicRenderBatches.empty()) {
            AX_CORE_LOG_WARN("No render batch for layer {}! Cannot add basic quad.", layer);
            return;
        }
        auto& basicVertices = renderLayers[layer].basicVertices;
        auto& currentBatch = renderLayers[layer].basicRenderBatches.back();

        if (currentBatch.vertexCount == 0) {
            currentBatch.vertexOffset = basicVertices.size();
        }

        if (basicVertices.size() + 4 > MAX_BASIC_QUADS * 4) {
            AX_CORE_LOG_WARN("Maximum basic quad count for layer {} reached! Cannot add more quads this frame.", layer);
            return;
        }
        Math::Vec4 params = Math::Vec4(size.x(), size.y(), 0.0f, 0.0f);
        // Top-Left
        basicVertices.push_back(UIVertex(pos, Math::Vec2(0.0f), color, params, radii));
        // Top-Right
        basicVertices.push_back(UIVertex(pos + Math::Vec2(size.x(), 0.0f), Math::Vec2(1.0f, 0.0f), color, params, radii));
        // Bottom-Right
        basicVertices.push_back(UIVertex(pos + size, Math::Vec2(1.0f), color, params, radii));
        // Bottom-Left
        basicVertices.push_back(UIVertex(pos + Math::Vec2(0.0f, size.y()), Math::Vec2(0.0f, 1.0f), color, params, radii));
        currentBatch.vertexCount += 4;
    }

    void UIRenderer::addDebugRect(const Math::Vec2& pos, const Math::Vec2& size, const Color& color) {
        float thickness = 1.0f;
        addBasicQuad(pos, Math::Vec2(size.x(), thickness), color);
        addBasicQuad(pos + Math::Vec2(0.0f, size.y() - thickness), Math::Vec2(size.x(), thickness), color);
        addBasicQuad(pos, Math::Vec2(thickness, size.y()), color);
        addBasicQuad(pos + Math::Vec2(size.x() - thickness, 0.0f), Math::Vec2(thickness, size.y()), color);
    }

    void UIRenderer::addFontQuad(const Math::Vec2& pos, const Math::Vec2& size, const Math::Vec2& uv0, const Math::Vec2& uv1, const Color& color,
                                 uint8_t layer) {
        if (renderLayers[layer].fontRenderBatches.empty()) {
            AX_CORE_LOG_WARN("No render batch for layer {}! Cannot add font quad.", layer);
            return;
        }
        auto& fontVertices = renderLayers[layer].fontVertices;
        auto& currentBatch = renderLayers[layer].fontRenderBatches.back();

        if (currentBatch.vertexCount == 0) {
            currentBatch.vertexOffset = fontVertices.size();
        }

        if (fontVertices.size() + 4 > MAX_FONT_QUADS * 4) {
            AX_CORE_LOG_WARN("Maximum font quad count for layer {} reached! Cannot add more quads this frame.", layer);
            return;
        }

        // Pushing vertices for an indexed quad (0,1,2, 2,3,0)
        fontVertices.push_back(UIVertex(pos, uv0, color, Math::Vec4::zero()));
        fontVertices.push_back(UIVertex(pos + Math::Vec2(size.x(), 0.0f), Math::Vec2(uv1.x(), uv0.y()), color, Math::Vec4::zero()));
        fontVertices.push_back(UIVertex(pos + size, uv1, color, Math::Vec4::zero()));
        fontVertices.push_back(UIVertex(pos + Math::Vec2(0.0f, size.y()), Math::Vec2(uv0.x(), uv1.y()), color, Math::Vec4::zero()));
        currentBatch.vertexCount += 4;
    }

    void UIRenderer::addText(const std::string& text, const Math::Vec2& pos, float fontSize, const Color& color, uint8_t layer) {
        const auto& asciiAtlas = openSansFont.getAsciiAtlas();

        float pixelSize = fontSize * (96.0f / 72.0f);

        float fontScale = pixelSize / asciiAtlas.unitsPerEm;
        float lineSpacing = pixelSize * 1.2f;

        float cursorX = pos.x();

        float ascender = asciiAtlas.ascender != 0 ? asciiAtlas.ascender : asciiAtlas.unitsPerEm * 0.8f;
        float cursorY = pos.y() + (ascender * fontScale);

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
                float minX = cursorX + (metrics.quadMin.x() * fontScale);
                float minY = cursorY - (metrics.quadMax.y() * fontScale);
                float maxX = cursorX + (metrics.quadMax.x() * fontScale);
                float maxY = cursorY - (metrics.quadMin.y() * fontScale);

                addFontQuad(Math::Vec2(minX, minY), Math::Vec2(maxX - minX, maxY - minY), metrics.uv0, metrics.uv1, color, layer);
            }

            cursorX += (metrics.advance * fontScale);
        }
    }

    void UIRenderer::addImageQuad(const Math::Vec2& pos, const Math::Vec2& size, Texture* texture, uint8_t layer) {
        auto& imageDrawCommands = renderLayers[layer].imageDrawCommands;

        if (imageDrawCommands.size() + 1 > MAX_IMAGE_QUADS) {
            AX_CORE_LOG_WARN("Maximum image quad count for layer {} reached! Cannot add more quads this frame.", layer);
            return;
        }
        if (scissorRectStack.empty()) {
            imageDrawCommands.push_back({texture, pos, size, Math::Rect()});
        } else {
            imageDrawCommands.push_back({texture, pos, size, scissorRectStack.top()});
        }
    }

    void UIRenderer::pushScissorRect(const Math::Rect& rect, uint8_t layer) {
        float dpi = window.getWindowDPI() / 96.0f;
        Math::Rect scaledRect = Math::Rect(rect.getPos() * dpi, rect.getSize() * dpi);

        scissorRectStack.push(scaledRect);
        renderLayers[layer].basicRenderBatches.push_back({0, 0, scaledRect});
        renderLayers[layer].fontRenderBatches.push_back({0, 0, scaledRect});
    }

    void UIRenderer::popScissorRect(uint8_t layer) {
        if (!scissorRectStack.empty()) {
            scissorRectStack.pop();
        }
        renderLayers[layer].basicRenderBatches.push_back({0, 0, scissorRectStack.empty() ? Math::Rect() : scissorRectStack.top()});
        renderLayers[layer].fontRenderBatches.push_back({0, 0, scissorRectStack.empty() ? Math::Rect() : scissorRectStack.top()});
    }

    float UIRenderer::calculateTextWidth(const std::string& text, float fontSize) {
        float pixelSize = fontSize * (96.0f / 72.0f);
        float fontScale = pixelSize / openSansFont.getAsciiAtlas().unitsPerEm;

        float maxWidth = 0.0f;
        float currentLineWidth = 0.0f;

        for (char c : text) {
            if (c == '\n') {
                if (currentLineWidth > maxWidth) {
                    maxWidth = currentLineWidth;
                }
                currentLineWidth = 0.0f;
                continue;
            }
            auto glyphIt = openSansFont.getAsciiAtlas().glyphs.find(static_cast<uint32_t>(c));
            if (glyphIt == openSansFont.getAsciiAtlas().glyphs.end()) {
                continue;
            }
            currentLineWidth += (glyphIt->second.advance * fontScale);
        }

        if (currentLineWidth > maxWidth) {
            maxWidth = currentLineWidth;
        }
        return maxWidth;
    }

    float UIRenderer::calculateTextHeight(float fontSize) {
        const auto& asciiAtlas = openSansFont.getAsciiAtlas();
        float pixelSize = fontSize * (96.0f / 72.0f);
        float fontScale = pixelSize / asciiAtlas.unitsPerEm;

        float ascender = asciiAtlas.ascender != 0 ? asciiAtlas.ascender : asciiAtlas.unitsPerEm * 0.8f;
        float descender = asciiAtlas.descender != 0 ? asciiAtlas.descender : -asciiAtlas.unitsPerEm * 0.2f;

        return (ascender - descender) * fontScale;
    }

    void UIRenderer::onRender(CommandBuffer* commandBuffer, Texture* renderTarget) {
        Math::Vec2 windowSize = Math::Vec2(window.getWidth(), window.getHeight());
        Math::iVec2 renderTargetSize = renderTarget->getSize();
        Math::Mat4 projection = Math::Mat4::orthographic(0.0f, windowSize.x(), 0.0f, windowSize.y(), -1.0f, 1.0f);

        std::vector<UIVertex> allBasicVertices;
        std::vector<UIVertex> allFontVertices;
        std::vector<UIVertex> allImageVertices;

        size_t totalBaisicVertexCount = 0;
        size_t totalFontVertexCount = 0;
        size_t totalImageVertexCount = 0;

        for (const auto& layer : renderLayers) {
            totalBaisicVertexCount += layer.basicVertices.size();
            totalFontVertexCount += layer.fontVertices.size();
            totalImageVertexCount += layer.imageDrawCommands.size() * 4; // 4 vertices per quad
        }

        allBasicVertices.reserve(totalBaisicVertexCount);
        allFontVertices.reserve(totalFontVertexCount);
        allImageVertices.reserve(totalImageVertexCount);

        for (auto& layer : renderLayers) {
            allBasicVertices.insert(allBasicVertices.end(), layer.basicVertices.begin(), layer.basicVertices.end());
            allFontVertices.insert(allFontVertices.end(), layer.fontVertices.begin(), layer.fontVertices.end());
            for (const auto& drawCommand : layer.imageDrawCommands) {
                allImageVertices.push_back(UIVertex(drawCommand.pos, Math::Vec2(0.0f), Color::white(), Math::Vec4::zero()));
                allImageVertices.push_back(
                    UIVertex(drawCommand.pos + Math::Vec2(drawCommand.size.x(), 0.0f), Math::Vec2(1.0f, 0.0f), Color::white(), Math::Vec4::zero()));
                allImageVertices.push_back(UIVertex(drawCommand.pos + drawCommand.size, Math::Vec2(1.0f), Color::white(), Math::Vec4::zero()));
                allImageVertices.push_back(
                    UIVertex(drawCommand.pos + Math::Vec2(0.0f, drawCommand.size.y()), Math::Vec2(0.0f, 1.0f), Color::white(), Math::Vec4::zero()));
            }
        }

        if (!allBasicVertices.empty()) {
            basicVertexBuffer->setData<UIVertex>(allBasicVertices);
        }
        if (!allFontVertices.empty()) {
            fontVertexBuffer->setData<UIVertex>(allFontVertices);
        }
        if (!allImageVertices.empty()) {
            imageVertexBuffer->setData<UIVertex>(allImageVertices);
        }

        size_t basicVertexOffset = 0;
        size_t fontVertexOffset = 0;
        size_t imageVertexOffset = 0;

        for (const auto& layer : renderLayers) {
            if (!layer.basicVertices.empty()) {
                basicRenderPass.colorAttachments[0].texture = renderTarget;
                basicRenderPass.width = renderTargetSize.x();
                basicRenderPass.height = renderTargetSize.y();

                commandBuffer->beginRendering(basicRenderPass);
                commandBuffer->bindPipeline(basicPipeline.get());
                commandBuffer->bindPushConstants(&projection, sizeof(projection));
                commandBuffer->bindVertexBuffers({basicVertexBuffer.get()});
                commandBuffer->bindIndexBuffer(basicIndexBuffer.get());
                commandBuffer->setViewport(0.0f, 0.0f, renderTargetSize.x(), renderTargetSize.y());

                for (const auto& batch : layer.basicRenderBatches) {
                    if (batch.vertexCount == 0) {
                        continue;
                    }
                    commandBuffer->setScissor(batch.scissorRect.x(), batch.scissorRect.y(), batch.scissorRect.width(), batch.scissorRect.height());
                    commandBuffer->drawIndexed(batch.vertexCount / 4 * 6, 1, 0, basicVertexOffset + batch.vertexOffset, 0);
                }

                commandBuffer->endRendering();
                basicVertexOffset += layer.basicVertices.size();
            }

            if (!layer.fontVertices.empty()) {
                fontRenderPass.colorAttachments[0].texture = renderTarget;
                fontRenderPass.width = renderTargetSize.x();
                fontRenderPass.height = renderTargetSize.y();

                commandBuffer->beginRendering(fontRenderPass);
                commandBuffer->bindPipeline(fontPipeline.get());
                commandBuffer->bindPushConstants(&projection, sizeof(projection));
                commandBuffer->bindResources({fontResourceSet.get()});
                commandBuffer->bindVertexBuffers({fontVertexBuffer.get()});
                commandBuffer->bindIndexBuffer(fontIndexBuffer.get());
                commandBuffer->setViewport(0.0f, 0.0f, renderTargetSize.x(), renderTargetSize.y());

                for (const auto& batch : layer.fontRenderBatches) {
                    if (batch.vertexCount == 0) {
                        continue;
                    }
                    commandBuffer->setScissor(batch.scissorRect.x(), batch.scissorRect.y(), batch.scissorRect.width(), batch.scissorRect.height());
                    commandBuffer->drawIndexed(batch.vertexCount / 4 * 6, 1, 0, fontVertexOffset + batch.vertexOffset, 0);
                }

                commandBuffer->endRendering();
                fontVertexOffset += layer.fontVertices.size();
            }

            if (!layer.imageDrawCommands.empty()) {
                imageRenderPass.colorAttachments[0].texture = renderTarget;
                imageRenderPass.width = renderTargetSize.x();
                imageRenderPass.height = renderTargetSize.y();

                commandBuffer->beginRendering(imageRenderPass);
                commandBuffer->bindPipeline(imagePipeline.get());
                commandBuffer->bindPushConstants(&projection, sizeof(projection));
                commandBuffer->bindVertexBuffers({imageVertexBuffer.get()});
                commandBuffer->bindIndexBuffer(imageIndexBuffer.get());
                commandBuffer->setViewport(0.0f, 0.0f, renderTargetSize.x(), renderTargetSize.y());

                for (const auto& drawCommand : layer.imageDrawCommands) {
                    commandBuffer->setScissor(drawCommand.scissorRect.x(), drawCommand.scissorRect.y(), drawCommand.scissorRect.width(),
                                              drawCommand.scissorRect.height());
                    ResourceSet* resourceSet = getResourceSetForTexture(drawCommand.texture);
                    commandBuffer->bindResources({resourceSet});
                    commandBuffer->drawIndexed(6, 1, 0, imageVertexOffset, 0);

                    imageVertexOffset += 4;
                }
                commandBuffer->endRendering();
            }
        }
    }

    void UIRenderer::createBasicRenderObjects() {
        basicShader = assetManager.getBuiltInShader("UI");

        Buffer::CreateInfo vertexBufferCreateInfo = {
            .size = sizeof(UIVertex) * 4 * MAX_BASIC_QUADS, .usage = BufferUsage::Vertex, .memoryUsage = MemoryUsage::GPUandCPU};
        basicVertexBuffer = rhi.createBuffer(vertexBufferCreateInfo);

        Buffer::CreateInfo indexBufferCreateInfo = {
            .size = sizeof(uint32_t) * 6 * MAX_BASIC_QUADS, .usage = BufferUsage::Index | BufferUsage::TransferDst, .memoryUsage = MemoryUsage::GPUOnly};
        basicIndexBuffer = rhi.createBuffer(indexBufferCreateInfo);

        Buffer::CreateInfo stagingBufferCreateInfo = {
            .size = sizeof(uint32_t) * 6 * MAX_BASIC_QUADS, .usage = BufferUsage::TransferSrc, .memoryUsage = MemoryUsage::GPUandCPU};
        std::unique_ptr<Buffer> stagingBuffer = rhi.createBuffer(stagingBufferCreateInfo);

        std::vector<uint32_t> indices(MAX_BASIC_QUADS * 6);
        uint32_t offset = 0;
        for (size_t i = 0; i < indices.size(); i += 6) {
            indices[i + 0] = offset + 0;
            indices[i + 1] = offset + 1;
            indices[i + 2] = offset + 2;
            indices[i + 3] = offset + 0;
            indices[i + 4] = offset + 2;
            indices[i + 5] = offset + 3;
            offset += 4;
        }
        stagingBuffer->setData<uint32_t>(indices);
        auto commandBuffer = rhi.beginSingleTimeCommands();
        commandBuffer->copyBuffer(stagingBuffer.get(), basicIndexBuffer.get(), stagingBufferCreateInfo.size);
        rhi.endSingleTimeCommands(commandBuffer.get());

        RenderAttachment colorAttachment{};
        colorAttachment.loadOp = LoadOp::Load;
        colorAttachment.storeOp = StoreOp::Store;
        colorAttachment.clearColor = Color::transparent();
        basicRenderPass.colorAttachments[0] = colorAttachment;
        basicRenderPass.colorAttachmentCount = 1;

        std::vector<VertexBindingDescription> vertexBindings = {{.binding = 0, .stride = sizeof(UIVertex), .inputRate = VertexInputRate::Vertex}};
        std::vector<VertexAttributeDescription> vertexAttributes = {
            {.location = 0, .binding = 0, .format = Format::R32G32Sfloat, .offset = offsetof(UIVertex, position)},
            {.location = 1, .binding = 0, .format = Format::R32G32Sfloat, .offset = offsetof(UIVertex, uv)},
            {.location = 2, .binding = 0, .format = Format::R32G32B32A32Sfloat, .offset = offsetof(UIVertex, color)},
            {.location = 3, .binding = 0, .format = Format::R32G32B32A32Sfloat, .offset = offsetof(UIVertex, params)},
            {.location = 4, .binding = 0, .format = Format::R32G32B32A32Sfloat, .offset = offsetof(UIVertex, radii)},
        };

        Pipeline::CreateInfo pipelineCreateInfo = {.shader = basicShader->getShader(),
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
                                                   .resourceLayouts = {}};
        basicPipeline = rhi.createPipeline(pipelineCreateInfo);
    }

    void UIRenderer::createFontRenderObjects() {
        fontShader = assetManager.getBuiltInShader("UIFont");

        Buffer::CreateInfo vertexBufferCreateInfo = {
            .size = sizeof(UIVertex) * 4 * MAX_FONT_QUADS, .usage = BufferUsage::Vertex, .memoryUsage = MemoryUsage::GPUandCPU};
        fontVertexBuffer = rhi.createBuffer(vertexBufferCreateInfo);

        Buffer::CreateInfo indexBufferCreateInfo = {
            .size = sizeof(uint32_t) * 6 * MAX_FONT_QUADS, .usage = BufferUsage::Index | BufferUsage::TransferDst, .memoryUsage = MemoryUsage::GPUOnly};
        fontIndexBuffer = rhi.createBuffer(indexBufferCreateInfo);

        Buffer::CreateInfo stagingBufferCreateInfo = {
            .size = sizeof(uint32_t) * 6 * MAX_FONT_QUADS, .usage = BufferUsage::TransferSrc, .memoryUsage = MemoryUsage::GPUandCPU};
        std::unique_ptr<Buffer> stagingBuffer = rhi.createBuffer(stagingBufferCreateInfo);

        std::vector<uint32_t> indices(MAX_FONT_QUADS * 6);
        uint32_t offset = 0;
        for (size_t i = 0; i < indices.size(); i += 6) {
            indices[i + 0] = offset + 0;
            indices[i + 1] = offset + 1;
            indices[i + 2] = offset + 2;
            indices[i + 3] = offset + 0;
            indices[i + 4] = offset + 2;
            indices[i + 5] = offset + 3;
            offset += 4;
        }

        stagingBuffer->setData<uint32_t>(indices);
        auto commandBuffer = rhi.beginSingleTimeCommands();
        commandBuffer->copyBuffer(stagingBuffer.get(), fontIndexBuffer.get(), stagingBufferCreateInfo.size);
        rhi.endSingleTimeCommands(commandBuffer.get());

        RenderAttachment colorAttachment{};
        colorAttachment.loadOp = LoadOp::Load;
        colorAttachment.storeOp = StoreOp::Store;
        colorAttachment.clearColor = Color::transparent();
        fontRenderPass.colorAttachments[0] = colorAttachment;
        fontRenderPass.colorAttachmentCount = 1;

        std::vector<VertexBindingDescription> vertexBindings = {{.binding = 0, .stride = sizeof(UIVertex), .inputRate = VertexInputRate::Vertex}};
        std::vector<VertexAttributeDescription> vertexAttributes = {
            {.location = 0, .binding = 0, .format = Format::R32G32Sfloat, .offset = offsetof(UIVertex, position)},
            {.location = 1, .binding = 0, .format = Format::R32G32Sfloat, .offset = offsetof(UIVertex, uv)},
            {.location = 2, .binding = 0, .format = Format::R32G32B32A32Sfloat, .offset = offsetof(UIVertex, color)},
        };

        std::vector<ResourceLayout::BindingCreateInfo> resourceLayoutBindings(2);
        resourceLayoutBindings[0].binding = 0;
        resourceLayoutBindings[0].type = ResourceType::Texture2D;
        resourceLayoutBindings[0].stages = ShaderStage::Fragment;
        resourceLayoutBindings[0].count = 1;
        resourceLayoutBindings[1].binding = 1;
        resourceLayoutBindings[1].type = ResourceType::Sampler;
        resourceLayoutBindings[1].stages = ShaderStage::Fragment;
        resourceLayoutBindings[1].count = 1;

        fontResourceLayout = rhi.createResourceLayout(resourceLayoutBindings);
        fontResourceSet = rhi.createResourceSet(fontResourceLayout.get());

        Texture* texturesData[] = {fontAtlasTexture.get()};
        Sampler* samplersData[] = {rhi.getNearestSampler()};
        std::vector<ResourceSet::Binding> resourceSetBindings(2);
        resourceSetBindings[0].binding = 0;
        resourceSetBindings[0].type = ResourceType::Texture2D;
        resourceSetBindings[0].stages = ShaderStage::Fragment;
        resourceSetBindings[0].data = texturesData;
        resourceSetBindings[1].binding = 1;
        resourceSetBindings[1].type = ResourceType::Sampler;
        resourceSetBindings[1].stages = ShaderStage::Fragment;
        resourceSetBindings[1].data = samplersData;

        fontResourceSet->update(resourceSetBindings);

        Pipeline::CreateInfo pipelineCreateInfo = {.shader = fontShader->getShader(),
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
                                                   .resourceLayouts = {fontResourceLayout.get()}};
        fontPipeline = rhi.createPipeline(pipelineCreateInfo);
    }

    void UIRenderer::createImageRenderObjects() {
        imageShader = assetManager.getBuiltInShader("UIImage");

        Buffer::CreateInfo vertexBufferCreateInfo = {.size = sizeof(UIVertex) * 4, .usage = BufferUsage::Vertex, .memoryUsage = MemoryUsage::GPUandCPU};
        imageVertexBuffer = rhi.createBuffer(vertexBufferCreateInfo);

        Buffer::CreateInfo indexBufferCreateInfo = {
            .size = sizeof(uint32_t) * 6, .usage = BufferUsage::Index | BufferUsage::TransferDst, .memoryUsage = MemoryUsage::GPUOnly};
        imageIndexBuffer = rhi.createBuffer(indexBufferCreateInfo);

        Buffer::CreateInfo stagingBufferCreateInfo = {.size = sizeof(uint32_t) * 6, .usage = BufferUsage::TransferSrc, .memoryUsage = MemoryUsage::GPUandCPU};
        std::unique_ptr<Buffer> stagingBuffer = rhi.createBuffer(stagingBufferCreateInfo);

        std::vector<uint32_t> indices = {0, 1, 2, 0, 2, 3};
        stagingBuffer->setData<uint32_t>(indices);

        auto commandBuffer = rhi.beginSingleTimeCommands();
        commandBuffer->copyBuffer(stagingBuffer.get(), imageIndexBuffer.get(), stagingBufferCreateInfo.size);
        rhi.endSingleTimeCommands(commandBuffer.get());

        RenderAttachment colorAttachment{};
        colorAttachment.loadOp = LoadOp::Load;
        colorAttachment.storeOp = StoreOp::Store;
        colorAttachment.clearColor = Color::transparent();
        imageRenderPass.colorAttachments[0] = colorAttachment;
        imageRenderPass.colorAttachmentCount = 1;

        std::vector<VertexBindingDescription> vertexBindings = {{.binding = 0, .stride = sizeof(UIVertex), .inputRate = VertexInputRate::Vertex}};
        std::vector<VertexAttributeDescription> vertexAttributes = {
            {.location = 0, .binding = 0, .format = Format::R32G32Sfloat, .offset = offsetof(UIVertex, position)},
            {.location = 1, .binding = 0, .format = Format::R32G32Sfloat, .offset = offsetof(UIVertex, uv)},
            {.location = 2, .binding = 0, .format = Format::R32G32B32A32Sfloat, .offset = offsetof(UIVertex, color)},
        };

        std::vector<ResourceLayout::BindingCreateInfo> resourceLayoutBindings(2);
        resourceLayoutBindings[0].binding = 0;
        resourceLayoutBindings[0].type = ResourceType::Texture2D;
        resourceLayoutBindings[0].stages = ShaderStage::Fragment;
        resourceLayoutBindings[0].count = 1;
        resourceLayoutBindings[1].binding = 1;
        resourceLayoutBindings[1].type = ResourceType::Sampler;
        resourceLayoutBindings[1].stages = ShaderStage::Fragment;
        resourceLayoutBindings[1].count = 1;

        imageResourceLayout = rhi.createResourceLayout(resourceLayoutBindings);

        Pipeline::CreateInfo pipelineCreateInfo = {.shader = imageShader->getShader(),
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
                                                   .resourceLayouts = {imageResourceLayout.get()}};
        imagePipeline = rhi.createPipeline(pipelineCreateInfo);
    }

    ResourceSet* UIRenderer::getResourceSetForTexture(Texture* texture) {
        auto it = imageResourceSets.find(texture);
        if (it != imageResourceSets.end()) {
            return it->second.get();
        }

        Texture* texturesData[] = {texture};
        Sampler* samplersData[] = {rhi.getNearestSampler()};
        std::vector<ResourceSet::Binding> resourceSetBindings(2);
        resourceSetBindings[0].binding = 0;
        resourceSetBindings[0].type = ResourceType::Texture2D;
        resourceSetBindings[0].stages = ShaderStage::Fragment;
        resourceSetBindings[0].data = texturesData;
        resourceSetBindings[1].binding = 1;
        resourceSetBindings[1].type = ResourceType::Sampler;
        resourceSetBindings[1].stages = ShaderStage::Fragment;
        resourceSetBindings[1].data = samplersData;
        std::unique_ptr<ResourceSet> resourceSet = rhi.createResourceSet(imageResourceLayout.get());
        resourceSet->update(resourceSetBindings);
        ResourceSet* resourceSetPtr = resourceSet.get();
        imageResourceSets[texture] = std::move(resourceSet);
        return resourceSetPtr;
    };
} // namespace Axiom
