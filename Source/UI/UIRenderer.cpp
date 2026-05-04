#include "UIRenderer.h"
#include "Core/Application.h"

namespace Axiom {
    UIRenderer::UIRenderer() {
        auto& asciiAtlas = openSansFont.getAsciiAtlas();
        Texture::CreateInfo fontAtlasCreateInfo = {.width = asciiAtlas.width,
                                                   .height = asciiAtlas.height,
                                                   .mipLevels = 1,
                                                   .arrayLayers = 1,
                                                   .format = Format::R8G8B8A8Unorm,
                                                   .usage = TextureUsage::Sampled | TextureUsage::TransferDst,
                                                   .aspect = TextureAspect::Color,
                                                   .initialState = TextureState::Undefined,
                                                   .memoryUsage = MemoryUsage::GPUOnly};
        fontAtlasTexture = Locator::getRenderer()->createTexture(fontAtlasCreateInfo);

        std::unique_ptr<Buffer> stagingBuffer = Locator::getRenderer()->createBuffer(
            {.size = asciiAtlas.width * asciiAtlas.height * asciiAtlas.channels, .usage = BufferUsage::TransferSrc, .memoryUsage = MemoryUsage::GPUandCPU});
        stagingBuffer->setData<uint8_t>(asciiAtlas.pixels);

        auto commandBuffer = Locator::getRenderer()->beginSingleTimeCommands();
        commandBuffer->copyBufferToTexture(stagingBuffer.get(), fontAtlasTexture.get(), asciiAtlas.width, asciiAtlas.height);
        Locator::getRenderer()->endSingleTimeCommands(commandBuffer.get());

        createBasicRenderObjects();
        createFontRenderObjects();
        createImageRenderObjects();
    }

    void UIRenderer::beginFrame() {
        basicVertices.clear();
        fontVertices.clear();
        imageDrawCommands.clear();
    }

    void UIRenderer::addBasicQuad(const Math::Vec2& pos, const Math::Vec2& size, const Color& color, const Math::Vec4& radii) {
        if (basicVertices.size() + 4 > MAX_BASIC_QUADS * 4) {
            AX_CORE_LOG_WARN("Maximum basic quad count reached! Cannot add more quads this frame.");
            return;
        }
        Math::Vec4 params = Math::Vec4(size.x(), size.y(), 0.0f, 0.0f);
        // Top-Left
        basicVertices.push_back(UIVertex(pos, Math::Vec2(0.0f), color, params, radii));
        // Bottom-Left
        basicVertices.push_back(UIVertex(pos + Math::Vec2(size.x(), 0.0f), Math::Vec2(1.0f, 0.0f), color, params, radii));
        // Bottom-Right
        basicVertices.push_back(UIVertex(pos + size, Math::Vec2(1.0f), color, params, radii));
        // Top-Right
        basicVertices.push_back(UIVertex(pos + Math::Vec2(0.0f, size.y()), Math::Vec2(0.0f, 1.0f), color, params, radii));
    }

    void UIRenderer::addDebugRect(const Math::Vec2& pos, const Math::Vec2& size, const Color& color) {
        float thickness = 1.0f;
        addBasicQuad(pos, Math::Vec2(size.x(), thickness), color);
        addBasicQuad(pos + Math::Vec2(0.0f, size.y() - thickness), Math::Vec2(size.x(), thickness), color);
        addBasicQuad(pos, Math::Vec2(thickness, size.y()), color);
        addBasicQuad(pos + Math::Vec2(size.x() - thickness, 0.0f), Math::Vec2(thickness, size.y()), color);
    }

    void UIRenderer::addFontQuad(const Math::Vec2& pos, const Math::Vec2& size, const Math::Vec2& uv0, const Math::Vec2& uv1, const Color& color) {
        if (fontVertices.size() + 4 > MAX_FONT_QUADS * 4) {
            AX_CORE_LOG_WARN("Maximum font quad count reached! Cannot add more quads this frame.");
            return;
        }

        // Pushing vertices for an indexed quad (0,1,2, 2,3,0)
        fontVertices.push_back(UIVertex(pos, uv0, color, Math::Vec4::zero()));
        fontVertices.push_back(UIVertex(pos + Math::Vec2(size.x(), 0.0f), Math::Vec2(uv1.x(), uv0.y()), color, Math::Vec4::zero()));
        fontVertices.push_back(UIVertex(pos + size, uv1, color, Math::Vec4::zero()));
        fontVertices.push_back(UIVertex(pos + Math::Vec2(0.0f, size.y()), Math::Vec2(uv0.x(), uv1.y()), color, Math::Vec4::zero()));
    }

    void UIRenderer::addText(const std::string& text, const Math::Vec2& pos, float fontSize, float dpiScale, const Color& color) {
        const auto& asciiAtlas = openSansFont.getAsciiAtlas();

        float pixelSize = fontSize * dpiScale;
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

                addFontQuad(Math::Vec2(minX, minY), Math::Vec2(maxX - minX, maxY - minY), metrics.uv0, metrics.uv1, color);
            }

            cursorX += (metrics.advance * fontScale);
        }
    }

    void UIRenderer::addImageQuad(const Math::Vec2& pos, const Math::Vec2& size, Texture* texture) {
        if (imageDrawCommands.size() + 1 > MAX_IMAGE_QUADS) {
            AX_CORE_LOG_WARN("Maximum image quad count reached! Cannot add more quads this frame.");
            return;
        }
        imageDrawCommands.push_back({texture, pos, size});
    }

    float UIRenderer::calculateTextWidth(const std::string& text, float fontSize, float dpiScale) {
        float pixelSize = fontSize * dpiScale;
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

    float UIRenderer::calculateTextHeight(float fontSize, float dpiScale) {
        const auto& asciiAtlas = openSansFont.getAsciiAtlas();
        float pixelSize = fontSize * dpiScale;
        float fontScale = pixelSize / asciiAtlas.unitsPerEm;

        float ascender = asciiAtlas.ascender != 0 ? asciiAtlas.ascender : asciiAtlas.unitsPerEm * 0.8f;
        float descender = asciiAtlas.descender != 0 ? asciiAtlas.descender : -asciiAtlas.unitsPerEm * 0.2f;

        return (ascender - descender) * fontScale;
    }

    void UIRenderer::onRender(CommandBuffer* commandBuffer, Texture* renderTarget) {
        Math::Vec2 windowSize = Math::Vec2(Locator::getWindow()->getWidth(), Locator::getWindow()->getHeight());
        Math::iVec2 renderTargetSize = renderTarget->getSize();
        Math::Mat4 projection = Math::Mat4::orthographic(0.0f, windowSize.x(), 0.0f, windowSize.y(), -1.0f, 1.0f);

        if (!basicVertices.empty()) {
            basicVertexBuffer->setData<UIVertex>(basicVertices);

            basicRenderPass.colorAttachments[0].texture = renderTarget;
            basicRenderPass.width = renderTargetSize.x();
            basicRenderPass.height = renderTargetSize.y();

            commandBuffer->beginRendering(basicRenderPass);
            commandBuffer->bindPipeline(basicPipeline.get());
            commandBuffer->bindPushConstants(&projection, sizeof(projection));
            commandBuffer->bindVertexBuffers({basicVertexBuffer.get()});
            commandBuffer->bindIndexBuffer(basicIndexBuffer.get());
            commandBuffer->setViewport(0.0f, 0.0f, renderTargetSize.x(), renderTargetSize.y());
            commandBuffer->setScissor(0, 0, renderTargetSize.x(), renderTargetSize.y());
            commandBuffer->drawIndexed(static_cast<uint32_t>(basicVertices.size() / 4) * 6, 1, 0, 0, 0);
            commandBuffer->endRendering();
        }

        if (!fontVertices.empty()) {
            fontVertexBuffer->setData<UIVertex>(fontVertices);

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
            commandBuffer->setScissor(0, 0, renderTargetSize.x(), renderTargetSize.y());
            commandBuffer->drawIndexed(static_cast<uint32_t>(fontVertices.size() / 4) * 6, 1, 0, 0, 0);
            commandBuffer->endRendering();
        }

        if (!imageDrawCommands.empty()) {
            imageRenderPass.colorAttachments[0].texture = renderTarget;
            imageRenderPass.width = renderTargetSize.x();
            imageRenderPass.height = renderTargetSize.y();

            commandBuffer->beginRendering(imageRenderPass);
            commandBuffer->bindPipeline(imagePipeline.get());
            commandBuffer->bindPushConstants(&projection, sizeof(projection));
            commandBuffer->setViewport(0.0f, 0.0f, renderTargetSize.x(), renderTargetSize.y());
            commandBuffer->setScissor(0, 0, renderTargetSize.x(), renderTargetSize.y());

            for (const auto& drawCommand : imageDrawCommands) {
                ResourceSet* resourceSet = getResourceSetForTexture(drawCommand.texture);
                commandBuffer->bindResources({resourceSet});

                std::vector<UIVertex> vertices = {
                    UIVertex(drawCommand.pos, Math::Vec2(0.0f), Color::white(), Math::Vec4::zero()),
                    UIVertex(drawCommand.pos + Math::Vec2(drawCommand.size.x(), 0.0f), Math::Vec2(1.0f, 0.0f), Color::white(), Math::Vec4::zero()),
                    UIVertex(drawCommand.pos + drawCommand.size, Math::Vec2(1.0f), Color::white(), Math::Vec4::zero()),
                    UIVertex(drawCommand.pos + Math::Vec2(0.0f, drawCommand.size.y()), Math::Vec2(0.0f, 1.0f), Color::white(), Math::Vec4::zero()),
                };
                imageVertexBuffer->setData<UIVertex>(vertices);
                commandBuffer->bindVertexBuffers({imageVertexBuffer.get()});
                commandBuffer->bindIndexBuffer(imageIndexBuffer.get());
                commandBuffer->drawIndexed(6, 1, 0, 0, 0);
            }
            commandBuffer->endRendering();
            imageDrawCommands.clear();
        }
    }

    void UIRenderer::createBasicRenderObjects() {
        UUID basicShaderHandle = AssetManager::importAsset("Assets/Shaders/BuiltIn.UI.axs", AssetType::Shader);
        basicShader = AssetManager::getAsset<ShaderAsset>(basicShaderHandle);

        Buffer::CreateInfo vertexBufferCreateInfo = {
            .size = sizeof(UIVertex) * 4 * MAX_BASIC_QUADS, .usage = BufferUsage::Vertex, .memoryUsage = MemoryUsage::GPUandCPU};
        basicVertexBuffer = Locator::getRenderer()->createBuffer(vertexBufferCreateInfo);

        Buffer::CreateInfo indexBufferCreateInfo = {
            .size = sizeof(uint32_t) * 6 * MAX_BASIC_QUADS, .usage = BufferUsage::Index | BufferUsage::TransferDst, .memoryUsage = MemoryUsage::GPUOnly};
        basicIndexBuffer = Locator::getRenderer()->createBuffer(indexBufferCreateInfo);

        Buffer::CreateInfo stagingBufferCreateInfo = {
            .size = sizeof(uint32_t) * 6 * MAX_BASIC_QUADS, .usage = BufferUsage::TransferSrc, .memoryUsage = MemoryUsage::GPUandCPU};
        std::unique_ptr<Buffer> stagingBuffer = Locator::getRenderer()->createBuffer(stagingBufferCreateInfo);

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
        auto commandBuffer = Locator::getRenderer()->beginSingleTimeCommands();
        commandBuffer->copyBuffer(stagingBuffer.get(), basicIndexBuffer.get(), stagingBufferCreateInfo.size);
        Locator::getRenderer()->endSingleTimeCommands(commandBuffer.get());

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
                                                   .colorAttachmentFormats = {Locator::getRenderer()->getRenderTargetFormat()},
                                                   .depthAttachmentFormat = Format::Undefined,
                                                   .resourceLayouts = {}};
        basicPipeline = Locator::getRenderer()->createPipeline(pipelineCreateInfo);
    }

    void UIRenderer::createFontRenderObjects() {
        UUID fontShaderHandle = AssetManager::importAsset("Assets/Shaders/BuiltIn.UI.Font.axs", AssetType::Shader);
        fontShader = AssetManager::getAsset<ShaderAsset>(fontShaderHandle);

        Buffer::CreateInfo vertexBufferCreateInfo = {
            .size = sizeof(UIVertex) * 4 * MAX_FONT_QUADS, .usage = BufferUsage::Vertex, .memoryUsage = MemoryUsage::GPUandCPU};
        fontVertexBuffer = Locator::getRenderer()->createBuffer(vertexBufferCreateInfo);

        Buffer::CreateInfo indexBufferCreateInfo = {
            .size = sizeof(uint32_t) * 6 * MAX_FONT_QUADS, .usage = BufferUsage::Index | BufferUsage::TransferDst, .memoryUsage = MemoryUsage::GPUOnly};
        fontIndexBuffer = Locator::getRenderer()->createBuffer(indexBufferCreateInfo);

        Buffer::CreateInfo stagingBufferCreateInfo = {
            .size = sizeof(uint32_t) * 6 * MAX_FONT_QUADS, .usage = BufferUsage::TransferSrc, .memoryUsage = MemoryUsage::GPUandCPU};
        std::unique_ptr<Buffer> stagingBuffer = Locator::getRenderer()->createBuffer(stagingBufferCreateInfo);

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
        auto commandBuffer = Locator::getRenderer()->beginSingleTimeCommands();
        commandBuffer->copyBuffer(stagingBuffer.get(), fontIndexBuffer.get(), stagingBufferCreateInfo.size);
        Locator::getRenderer()->endSingleTimeCommands(commandBuffer.get());

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
        resourceLayoutBindings[0].type = ResourceType::Texture;
        resourceLayoutBindings[0].stages = ShaderStage::Fragment;
        resourceLayoutBindings[0].count = 1;
        resourceLayoutBindings[1].binding = 1;
        resourceLayoutBindings[1].type = ResourceType::Sampler;
        resourceLayoutBindings[1].stages = ShaderStage::Fragment;
        resourceLayoutBindings[1].count = 1;

        std::vector<ResourceSet::Binding> resourceSetBindings(2);
        resourceSetBindings[0].binding = 0;
        resourceSetBindings[0].type = ResourceType::Texture;
        resourceSetBindings[0].textures = {fontAtlasTexture.get()};
        resourceSetBindings[1].binding = 1;
        resourceSetBindings[1].type = ResourceType::Sampler;
        resourceSetBindings[1].samplers = {Locator::getRenderer()->getNearestSampler()};

        fontResourceLayout = Locator::getRenderer()->createResourceLayout(resourceLayoutBindings);

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
                                                   .colorAttachmentFormats = {Locator::getRenderer()->getRenderTargetFormat()},
                                                   .depthAttachmentFormat = Format::Undefined,
                                                   .resourceLayouts = {fontResourceLayout.get()}};
        fontPipeline = Locator::getRenderer()->createPipeline(pipelineCreateInfo);
        fontResourceSet = fontPipeline->createResourceSet(fontResourceLayout.get());
        fontResourceSet->update(resourceSetBindings);
    }

    void UIRenderer::createImageRenderObjects() {
        UUID imageShaderHandle = AssetManager::importAsset("Assets/Shaders/BuiltIn.UI.Image.axs", AssetType::Shader);
        imageShader = AssetManager::getAsset<ShaderAsset>(imageShaderHandle);

        Buffer::CreateInfo vertexBufferCreateInfo = {.size = sizeof(UIVertex) * 4, .usage = BufferUsage::Vertex, .memoryUsage = MemoryUsage::GPUandCPU};
        imageVertexBuffer = Locator::getRenderer()->createBuffer(vertexBufferCreateInfo);

        Buffer::CreateInfo indexBufferCreateInfo = {
            .size = sizeof(uint32_t) * 6, .usage = BufferUsage::Index | BufferUsage::TransferDst, .memoryUsage = MemoryUsage::GPUOnly};
        imageIndexBuffer = Locator::getRenderer()->createBuffer(indexBufferCreateInfo);

        Buffer::CreateInfo stagingBufferCreateInfo = {.size = sizeof(uint32_t) * 6, .usage = BufferUsage::TransferSrc, .memoryUsage = MemoryUsage::GPUandCPU};
        std::unique_ptr<Buffer> stagingBuffer = Locator::getRenderer()->createBuffer(stagingBufferCreateInfo);

        std::vector<uint32_t> indices = {0, 1, 2, 0, 2, 3};
        stagingBuffer->setData<uint32_t>(indices);

        auto commandBuffer = Locator::getRenderer()->beginSingleTimeCommands();
        commandBuffer->copyBuffer(stagingBuffer.get(), imageIndexBuffer.get(), stagingBufferCreateInfo.size);
        Locator::getRenderer()->endSingleTimeCommands(commandBuffer.get());

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
        resourceLayoutBindings[0].type = ResourceType::Texture;
        resourceLayoutBindings[0].stages = ShaderStage::Fragment;
        resourceLayoutBindings[0].count = 1;
        resourceLayoutBindings[1].binding = 1;
        resourceLayoutBindings[1].type = ResourceType::Sampler;
        resourceLayoutBindings[1].stages = ShaderStage::Fragment;
        resourceLayoutBindings[1].count = 1;
        std::vector<ResourceSet::Binding> resourceSetBindings(2);
        resourceSetBindings[0].binding = 0;
        resourceSetBindings[0].type = ResourceType::Texture;
        resourceSetBindings[0].textures = {nullptr}; // will be set dynamically during rendering
        resourceSetBindings[1].binding = 1;
        resourceSetBindings[1].type = ResourceType::Sampler;
        resourceSetBindings[1].samplers = {Locator::getRenderer()->getNearestSampler()};

        imageResourceLayout = Locator::getRenderer()->createResourceLayout(resourceLayoutBindings);

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
                                                   .colorAttachmentFormats = {Locator::getRenderer()->getRenderTargetFormat()},
                                                   .depthAttachmentFormat = Format::Undefined,
                                                   .resourceLayouts = {imageResourceLayout.get()}};
        imagePipeline = Locator::getRenderer()->createPipeline(pipelineCreateInfo);
    }

    ResourceSet* UIRenderer::getResourceSetForTexture(Texture* texture) {
        auto it = imageResourceSets.find(texture);
        if (it != imageResourceSets.end()) {
            return it->second.get();
        }

        std::vector<ResourceSet::Binding> resourceSetBindings(2);
        resourceSetBindings[0].binding = 0;
        resourceSetBindings[0].type = ResourceType::Texture;
        resourceSetBindings[0].textures = {texture};
        resourceSetBindings[1].binding = 1;
        resourceSetBindings[1].type = ResourceType::Sampler;
        resourceSetBindings[1].samplers = {Locator::getRenderer()->getNearestSampler()};
        std::unique_ptr<ResourceSet> resourceSet = imagePipeline->createResourceSet(imageResourceLayout.get());
        resourceSet->update(resourceSetBindings);
        ResourceSet* resourceSetPtr = resourceSet.get();
        imageResourceSets[texture] = std::move(resourceSet);
        return resourceSetPtr;
    };
} // namespace Axiom