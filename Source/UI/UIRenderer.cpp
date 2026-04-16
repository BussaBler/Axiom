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
        fontAtlasTexture = Application::getRenderer()->createTexture(fontAtlasCreateInfo);

        std::unique_ptr<Buffer> stagingBuffer = Application::getRenderer()->createBuffer(
            {.size = asciiAtlas.width * asciiAtlas.height * asciiAtlas.channels, .usage = BufferUsage::TransferSrc, .memoryUsage = MemoryUsage::GPUandCPU});
        stagingBuffer->setData<uint8_t>(asciiAtlas.pixels);

        auto commandBuffer = Application::getRenderer()->beginSingleTimeCommands();
        commandBuffer->copyBufferToTexture(stagingBuffer.get(), fontAtlasTexture.get(), asciiAtlas.width, asciiAtlas.height);
        Application::getRenderer()->endSingleTimeCommands(commandBuffer.get());

        createBasicRenderObjects();
        createFontRenderObjects();
    }

    void UIRenderer::clearRenderData() {
        basicVertices.clear();
        fontVertices.clear();
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
        fontVertices.push_back(UIVertex(pos, uv0, color, Math::Vec4::zero()));
        fontVertices.push_back(UIVertex(pos + Math::Vec2(size.x(), 0.0f), Math::Vec2(uv1.x(), uv0.y()), color, Math::Vec4::zero()));
        fontVertices.push_back(UIVertex(pos + size, uv1, color, Math::Vec4::zero()));
        fontVertices.push_back(UIVertex(pos + Math::Vec2(0.0f, size.y()), Math::Vec2(uv0.x(), uv1.y()), color, Math::Vec4::zero()));
    }

    void UIRenderer::addImageQuad(const Math::Vec2& pos, const Math::Vec2& size, const Texture* texture) {
        if (imageDrawCommands.size() + 1 > MAX_IMAGE_QUADS) {
            AX_CORE_LOG_WARN("Maximum image quad count reached! Cannot add more quads this frame.");
            return;
        }
        imageDrawCommands.push_back({texture, pos, size});
    }

    void UIRenderer::drawUIElements(CommandBuffer* commandBuffer) {
        Math::Vec2 windowSize = Math::Vec2(Application::getWindow()->getWidth(), Application::getWindow()->getHeight());
        Math::Vec2 framebufferSize = Math::Vec2(Application::getWindow()->getFramebufferWidth(), Application::getWindow()->getFramebufferHeight());
        Math::Mat4 projection = Math::Mat4::orthographic(0.0f, windowSize.x(), 0.0f, windowSize.y(), -1.0f, 1.0f);

        if (!basicVertices.empty()) {
            basicVertexBuffer->setData<UIVertex>(basicVertices);

            basicRenderPass.colorAttachments[0].texture = Application::getRenderer()->getCurrentRenderTarget();
            basicRenderPass.width = static_cast<uint32_t>(framebufferSize.x());
            basicRenderPass.height = static_cast<uint32_t>(framebufferSize.y());

            commandBuffer->beginRendering(basicRenderPass);
            commandBuffer->bindPipeline(basicPipeline.get());
            commandBuffer->bindPushConstants(&projection, sizeof(projection));
            commandBuffer->bindVertexBuffers({basicVertexBuffer.get()});
            commandBuffer->bindIndexBuffer(basicIndexBuffer.get());
            commandBuffer->setViewport(0.0f, 0.0f, framebufferSize.x(), framebufferSize.y());
            commandBuffer->setScissor(0, 0, framebufferSize.x(), framebufferSize.y());
            commandBuffer->drawIndexed(static_cast<uint32_t>(basicVertices.size() / 4) * 6, 1, 0, 0, 0);
            commandBuffer->endRendering();
        }

        if (!fontVertices.empty()) {
            fontVertexBuffer->setData<UIVertex>(fontVertices);

            fontRenderPass.colorAttachments[0].texture = Application::getRenderer()->getCurrentRenderTarget();
            fontRenderPass.width = static_cast<uint32_t>(framebufferSize.x());
            fontRenderPass.height = static_cast<uint32_t>(framebufferSize.y());

            commandBuffer->beginRendering(fontRenderPass);
            commandBuffer->bindPipeline(fontPipeline.get());
            commandBuffer->bindPushConstants(&projection, sizeof(projection));
            commandBuffer->bindResources({fontResourceSet.get()});
            commandBuffer->bindVertexBuffers({fontVertexBuffer.get()});
            commandBuffer->bindIndexBuffer(fontIndexBuffer.get());
            commandBuffer->setViewport(0.0f, 0.0f, framebufferSize.x(), framebufferSize.y());
            commandBuffer->setScissor(0, 0, framebufferSize.x(), framebufferSize.y());
            commandBuffer->drawIndexed(static_cast<uint32_t>(fontVertices.size() / 4) * 6, 1, 0, 0, 0);
            commandBuffer->endRendering();
        }

        if (!imageDrawCommands.empty()) {
            commandBuffer->beginRendering(imageRenderPass);
            commandBuffer->bindPipeline(imagePipeline.get());
            commandBuffer->bindPushConstants(&projection, sizeof(projection));
            commandBuffer->setViewport(0.0f, 0.0f, framebufferSize.x(), framebufferSize.y());
            commandBuffer->setScissor(0, 0, framebufferSize.x(), framebufferSize.y());

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
        Buffer::CreateInfo vertexBufferCreateInfo = {
            .size = sizeof(UIVertex) * 4 * MAX_BASIC_QUADS, .usage = BufferUsage::Vertex, .memoryUsage = MemoryUsage::GPUandCPU};
        basicVertexBuffer = Application::getRenderer()->createBuffer(vertexBufferCreateInfo);

        Buffer::CreateInfo indexBufferCreateInfo = {
            .size = sizeof(uint32_t) * 6 * MAX_BASIC_QUADS, .usage = BufferUsage::Index | BufferUsage::TransferDst, .memoryUsage = MemoryUsage::GPUOnly};
        basicIndexBuffer = Application::getRenderer()->createBuffer(indexBufferCreateInfo);

        Buffer::CreateInfo stagingBufferCreateInfo = {
            .size = sizeof(uint32_t) * 6 * MAX_BASIC_QUADS, .usage = BufferUsage::TransferSrc, .memoryUsage = MemoryUsage::GPUandCPU};
        std::unique_ptr<Buffer> stagingBuffer = Application::getRenderer()->createBuffer(stagingBufferCreateInfo);

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
        auto commandBuffer = Application::getRenderer()->beginSingleTimeCommands();
        commandBuffer->copyBuffer(stagingBuffer.get(), basicIndexBuffer.get(), stagingBufferCreateInfo.size);
        Application::getRenderer()->endSingleTimeCommands(commandBuffer.get());

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

        Pipeline::CreateInfo pipelineCreateInfo = {//.vertexShaderPath = "Assets/Shaders/BuiltIn.UI.vert",
                                                   //.fragmentShaderPath = "Assets/Shaders/BuiltIn.UI.frag",
                                                   .uniqueShaderPath = "Assets/Shaders/BuiltIn.UI.metal",
                                                   .vertexBindings = vertexBindings,
                                                   .vertexAttributes = vertexAttributes,
                                                   .topology = PrimitiveTopology::TriangleList,
                                                   .polygonMode = PolygonMode::Fill,
                                                   .cullMode = CullMode::None,
                                                   .frontFaceClockwise = true,
                                                   .enableBlending = true,
                                                   .enableDepthTest = false,
                                                   .enableDepthWrite = false,
                                                   .colorAttachmentFormats = {Application::getRenderer()->getRenderTargetFormat()},
                                                   .depthAttachmentFormat = Format::Undefined,
                                                   .resourceLayouts = {}};
        basicPipeline = Application::getRenderer()->createPipeline(pipelineCreateInfo);
    }

    void UIRenderer::createFontRenderObjects() {
        Buffer::CreateInfo vertexBufferCreateInfo = {
            .size = sizeof(UIVertex) * 4 * MAX_FONT_QUADS, .usage = BufferUsage::Vertex, .memoryUsage = MemoryUsage::GPUandCPU};
        fontVertexBuffer = Application::getRenderer()->createBuffer(vertexBufferCreateInfo);

        Buffer::CreateInfo indexBufferCreateInfo = {
            .size = sizeof(uint32_t) * 6 * MAX_FONT_QUADS, .usage = BufferUsage::Index | BufferUsage::TransferDst, .memoryUsage = MemoryUsage::GPUOnly};
        fontIndexBuffer = Application::getRenderer()->createBuffer(indexBufferCreateInfo);

        Buffer::CreateInfo stagingBufferCreateInfo = {
            .size = sizeof(uint32_t) * 6 * MAX_FONT_QUADS, .usage = BufferUsage::TransferSrc, .memoryUsage = MemoryUsage::GPUandCPU};
        std::unique_ptr<Buffer> stagingBuffer = Application::getRenderer()->createBuffer(stagingBufferCreateInfo);

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
        auto commandBuffer = Application::getRenderer()->beginSingleTimeCommands();
        commandBuffer->copyBuffer(stagingBuffer.get(), fontIndexBuffer.get(), stagingBufferCreateInfo.size);
        Application::getRenderer()->endSingleTimeCommands(commandBuffer.get());

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
        resourceSetBindings[0].texture = fontAtlasTexture.get();
        resourceSetBindings[1].binding = 1;
        resourceSetBindings[1].type = ResourceType::Sampler;
        resourceSetBindings[1].sampler = Application::getRenderer()->getNearestSampler();

        fontResourceLayout = Application::getRenderer()->createResourceLayout(resourceLayoutBindings);

        Pipeline::CreateInfo pipelineCreateInfo = {//.vertexShaderPath = "Assets/Shaders/BuiltIn.UI.Font.vert",
                                                   //.fragmentShaderPath = "Assets/Shaders/BuiltIn.UI.Font.frag",
                                                   .uniqueShaderPath = "Assets/Shaders/BuiltIn.UI.Font.metal",
                                                   .vertexBindings = vertexBindings,
                                                   .vertexAttributes = vertexAttributes,
                                                   .topology = PrimitiveTopology::TriangleList,
                                                   .polygonMode = PolygonMode::Fill,
                                                   .cullMode = CullMode::None,
                                                   .frontFaceClockwise = true,
                                                   .enableBlending = true,
                                                   .enableDepthTest = false,
                                                   .enableDepthWrite = false,
                                                   .colorAttachmentFormats = {Application::getRenderer()->getRenderTargetFormat()},
                                                   .depthAttachmentFormat = Format::Undefined,
                                                   .resourceLayouts = {fontResourceLayout.get()}};
        fontPipeline = Application::getRenderer()->createPipeline(pipelineCreateInfo);
        fontResourceSet = fontPipeline->createResourceSet(fontResourceLayout.get());
        fontResourceSet->update(resourceSetBindings);
    }

    void UIRenderer::createImageRenderObjects() {
        Buffer::CreateInfo vertexBufferCreateInfo = {.size = sizeof(UIVertex) * 4, .usage = BufferUsage::Vertex, .memoryUsage = MemoryUsage::GPUandCPU};
        imageVertexBuffer = Application::getRenderer()->createBuffer(vertexBufferCreateInfo);

        Buffer::CreateInfo indexBufferCreateInfo = {
            .size = sizeof(uint32_t) * 6, .usage = BufferUsage::Index | BufferUsage::TransferDst, .memoryUsage = MemoryUsage::GPUOnly};
        imageIndexBuffer = Application::getRenderer()->createBuffer(indexBufferCreateInfo);

        Buffer::CreateInfo stagingBufferCreateInfo = {.size = sizeof(uint32_t) * 6, .usage = BufferUsage::TransferSrc, .memoryUsage = MemoryUsage::GPUandCPU};
        std::unique_ptr<Buffer> stagingBuffer = Application::getRenderer()->createBuffer(stagingBufferCreateInfo);

        std::vector<uint32_t> indices = {0, 1, 2, 0, 2, 3};
        stagingBuffer->setData<uint32_t>(indices);

        auto commandBuffer = Application::getRenderer()->beginSingleTimeCommands();
        commandBuffer->copyBuffer(stagingBuffer.get(), imageIndexBuffer.get(), stagingBufferCreateInfo.size);
        Application::getRenderer()->endSingleTimeCommands(commandBuffer.get());

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
        resourceSetBindings[0].texture = nullptr; // will be set dynamically during rendering
        resourceSetBindings[1].binding = 1;
        resourceSetBindings[1].type = ResourceType::Sampler;
        resourceSetBindings[1].sampler = Application::getRenderer()->getLinearSampler();

        imageResourceLayout = Application::getRenderer()->createResourceLayout(resourceLayoutBindings);

        Pipeline::CreateInfo pipelineCreateInfo = {//.vertexShaderPath = "Assets/Shaders/BuiltIn.UI.Image.vert",
                                                   //.fragmentShaderPath = "Assets/Shaders/BuiltIn.UI.Image.frag",
                                                   .uniqueShaderPath = "Assets/Shaders/BuiltIn.UI.Image.metal",
                                                   .vertexBindings = vertexBindings,
                                                   .vertexAttributes = vertexAttributes,
                                                   .topology = PrimitiveTopology::TriangleList,
                                                   .polygonMode = PolygonMode::Fill,
                                                   .cullMode = CullMode::None,
                                                   .frontFaceClockwise = true,
                                                   .enableBlending = true,
                                                   .enableDepthTest = false,
                                                   .enableDepthWrite = false,
                                                   .colorAttachmentFormats = {Application::getRenderer()->getRenderTargetFormat()},
                                                   .depthAttachmentFormat = Format::Undefined,
                                                   .resourceLayouts = {imageResourceLayout.get()}};
        imagePipeline = Application::getRenderer()->createPipeline(pipelineCreateInfo);
    }

    ResourceSet* UIRenderer::getResourceSetForTexture(const Texture* texture) {
        // In a real implementation, you would want to cache resource sets for each texture to avoid creating a new one every frame.
        std::vector<ResourceSet::Binding> resourceSetBindings(2);
        resourceSetBindings[0].binding = 0;
        resourceSetBindings[0].type = ResourceType::Texture;
        resourceSetBindings[0].texture = texture;
        resourceSetBindings[1].binding = 1;
        resourceSetBindings[1].type = ResourceType::Sampler;
        resourceSetBindings[1].sampler = Application::getRenderer()->getLinearSampler();

        std::unique_ptr<ResourceSet> resourceSet = imagePipeline->createResourceSet(imageResourceLayout.get());
        resourceSet->update(resourceSetBindings);
        return resourceSet.release();
    };
} // namespace Axiom