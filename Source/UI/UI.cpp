#include "UI.h"
#include "Core/Application.h"

namespace Axiom {
    UI *UI::instance = nullptr;

    UI::UI() {
        auto &asciiAtlas = jetBrainsMonoFont.getAsciiAtlas();
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

        createBaseRenderObjects();
        createFontRenderObjects();
    }

    void UI::init() {
        AX_CORE_LOG_INFO("Initializing UI system...");
        if (instance) {
            AX_CORE_LOG_WARN("UI system already initialized!");
            return;
        }
        instance = new UI();
    }

    void UI::shutdown() {
        AX_CORE_LOG_INFO("Shutting down UI system...");
        if (instance == nullptr) {
            AX_CORE_LOG_WARN("UI system not initialized!");
            return;
        }
        delete instance;
        instance = nullptr;
    }

    void UI::beginFrame() {
        instance->hotItem = 0;
        instance->baseVertices.clear();
        instance->fontVertices.clear();
    }

    void UI::endFrame() {
        if (!instance->isMouseButtonOneDown) {
            instance->activeItem = 0;
        }
        instance->shouldConsumeMouse = (instance->hotItem != 0 || instance->activeItem != 0);
    }

    void UI::render(CommandBuffer *commandBuffer) {
        Math::Vec2 renderTargetSize = Application::getRenderer()->getCurrentRenderTargetSize();
        Math::Mat4 projection = Math::Mat4::orthographic(0.0f, renderTargetSize.x(), 0.0f, renderTargetSize.y(), -1.0f, 1.0f);

        if (!instance->baseVertices.empty()) {
            instance->baseVertexBuffer->setData<UIVertex>(instance->baseVertices);

            instance->baseRenderPass.colorAttachments[0].texture = Application::getRenderer()->getCurrentRenderTarget();
            instance->baseRenderPass.width = static_cast<uint32_t>(renderTargetSize.x());
            instance->baseRenderPass.height = static_cast<uint32_t>(renderTargetSize.y());

            commandBuffer->beginRendering(instance->baseRenderPass);
            commandBuffer->bindPipeline(instance->basePipeline.get());
            commandBuffer->bindPushConstants(&projection, sizeof(projection));
            commandBuffer->bindVertexBuffers({instance->baseVertexBuffer.get()});
            commandBuffer->bindIndexBuffer(instance->baseIndexBuffer.get());
            commandBuffer->setViewport(0.0f, 0.0f, renderTargetSize.x(), renderTargetSize.y());
            commandBuffer->setScissor(0, 0, renderTargetSize.x(), renderTargetSize.y());
            commandBuffer->drawIndexed(static_cast<uint32_t>(instance->baseVertices.size() / 4) * 6, 1, 0, 0, 0);
            commandBuffer->endRendering();
        }

        if (!instance->fontVertices.empty()) {
            instance->fontVertexBuffer->setData<UIVertex>(instance->fontVertices);

            instance->fontRenderPass.colorAttachments[0].texture = Application::getRenderer()->getCurrentRenderTarget();
            instance->fontRenderPass.width = static_cast<uint32_t>(renderTargetSize.x());
            instance->fontRenderPass.height = static_cast<uint32_t>(renderTargetSize.y());

            commandBuffer->beginRendering(instance->fontRenderPass);
            commandBuffer->bindPipeline(instance->fontPipeline.get());
            commandBuffer->bindPushConstants(&projection, sizeof(projection));
            commandBuffer->bindResources({instance->fontResourceSet.get()});
            commandBuffer->bindVertexBuffers({instance->fontVertexBuffer.get()});
            commandBuffer->bindIndexBuffer(instance->fontIndexBuffer.get());
            commandBuffer->setViewport(0.0f, 0.0f, renderTargetSize.x(), renderTargetSize.y());
            commandBuffer->setScissor(0, 0, renderTargetSize.x(), renderTargetSize.y());
            commandBuffer->drawIndexed(static_cast<uint32_t>(instance->fontVertices.size() / 4) * 6, 1, 0, 0, 0);
            commandBuffer->endRendering();
        }
    }

    void UI::setMousePosition(Math::Vec2 pos) {
        instance->mousePosition = pos;
    }

    void UI::setMouseButtonState(KeyCode button, bool pressed) {
        switch (button) {
        case KeyCode::LeftButton:
            instance->isMouseButtonOneDown = pressed;
            break;
        case KeyCode::RightButton:
            instance->isMouseButtonTwoDown = pressed;
        default:
            break;
        }
    }

    bool UI::shouldConsumeMouseEvents() {
        return instance->shouldConsumeMouse;
    }

    void UI::addBaseQuad(const Math::Vec2 &pos, const Math::Vec2 &size, const Math::Vec4 &color) {
        // Top-Left
        baseVertices.push_back(UIVertex(pos, Math::Vec2(0.0f), color));
        // Bottom-Left
        baseVertices.push_back(UIVertex(pos + Math::Vec2(size.x(), 0.0f), Math::Vec2(1.0f, 0.0f), color));
        // Bottom-Right
        baseVertices.push_back(UIVertex(pos + size, Math::Vec2(1.0f), color));
        // Top-Right
        baseVertices.push_back(UIVertex(pos + Math::Vec2(0.0f, size.y()), Math::Vec2(0.0f, 1.0f), color));
    }

    void UI::addFontQuad(const Math::Vec2 &pos, const Math::Vec2 &size, const Math::Vec2 &uv0, const Math::Vec2 &uv1, const Math::Vec4 &color) {
        fontVertices.push_back(UIVertex(pos, uv0, color));
        fontVertices.push_back(UIVertex(pos + Math::Vec2(size.x(), 0.0f), Math::Vec2(uv1.x(), uv0.y()), color));
        fontVertices.push_back(UIVertex(pos + size, uv1, color));
        fontVertices.push_back(UIVertex(pos + Math::Vec2(0.0f, size.y()), Math::Vec2(uv0.x(), uv1.y()), color));
    }

    void UI::createBaseRenderObjects() {
        Buffer::CreateInfo vertexBufferCreateInfo = {
            .size = sizeof(UIVertex) * 4 * maxButtonQuads, .usage = BufferUsage::Vertex, .memoryUsage = MemoryUsage::GPUandCPU};
        baseVertexBuffer = Application::getRenderer()->createBuffer(vertexBufferCreateInfo);

        Buffer::CreateInfo indexBufferCreateInfo = {
            .size = sizeof(uint32_t) * 6 * maxButtonQuads, .usage = BufferUsage::Index | BufferUsage::TransferDst, .memoryUsage = MemoryUsage::GPUOnly};
        baseIndexBuffer = Application::getRenderer()->createBuffer(indexBufferCreateInfo);

        Buffer::CreateInfo stagingBufferCreateInfo = {
            .size = sizeof(uint32_t) * 6 * maxButtonQuads, .usage = BufferUsage::TransferSrc, .memoryUsage = MemoryUsage::GPUandCPU};
        std::unique_ptr<Buffer> stagingBuffer = Application::getRenderer()->createBuffer(stagingBufferCreateInfo);

        std::vector<uint32_t> indices(maxButtonQuads * 6);
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
        commandBuffer->copyBuffer(stagingBuffer.get(), baseIndexBuffer.get(), stagingBufferCreateInfo.size);
        Application::getRenderer()->endSingleTimeCommands(commandBuffer.get());

        RenderAttachment colorAttachment{};
        colorAttachment.loadOp = LoadOp::Load;
        colorAttachment.storeOp = StoreOp::Store;
        colorAttachment.clearColor = Math::Vec4(0.0f);
        baseRenderPass.colorAttachments[0] = colorAttachment;
        baseRenderPass.colorAttachmentCount = 1;

        std::vector<VertexBindingDescription> vertexBindings = {{.binding = 0, .stride = sizeof(UIVertex), .inputRate = VertexInputRate::Vertex}};
        std::vector<VertexAttributeDescription> vertexAttributes = {
            {.location = 0, .binding = 0, .format = Format::R32G32Sfloat, .offset = offsetof(UIVertex, position)},
            {.location = 1, .binding = 0, .format = Format::R32G32Sfloat, .offset = offsetof(UIVertex, uv)},
            {.location = 2, .binding = 0, .format = Format::R32G32B32A32Sfloat, .offset = offsetof(UIVertex, color)},
        };

        Pipeline::CreateInfo pipelineCreateInfo = {.vertexShaderPath = "Assets/Shaders/BuiltIn.UI.vert",
                                                   .fragmentShaderPath = "Assets/Shaders/BuiltIn.UI.frag",
                                                   .vertexBindings = vertexBindings,
                                                   .vertexAttributes = vertexAttributes,
                                                   .topology = PrimitiveTopology::TriangleList,
                                                   .polygonMode = PolygonMode::Fill,
                                                   .cullMode = CullMode::None,
                                                   .frontFaceClockwise = true,
                                                   .enableBlending = true,
                                                   .enableDepthTest = false,
                                                   .enableDepthWrite = false,
                                                   .colorAttachmentFormats = {Application::getRenderer()->getCurrentRenderTarget()->getFormat()},
                                                   .depthAttachmentFormat = Format::Undefined,
                                                   .resourceLayouts = {}};
        basePipeline = Application::getRenderer()->createPipeline(pipelineCreateInfo);
    }

    void UI::createFontRenderObjects() {
        Buffer::CreateInfo vertexBufferCreateInfo = {
            .size = sizeof(UIVertex) * 4 * maxFontQuads, .usage = BufferUsage::Vertex, .memoryUsage = MemoryUsage::GPUandCPU};
        fontVertexBuffer = Application::getRenderer()->createBuffer(vertexBufferCreateInfo);

        Buffer::CreateInfo indexBufferCreateInfo = {
            .size = sizeof(uint32_t) * 6 * maxFontQuads, .usage = BufferUsage::Index | BufferUsage::TransferDst, .memoryUsage = MemoryUsage::GPUOnly};
        fontIndexBuffer = Application::getRenderer()->createBuffer(indexBufferCreateInfo);

        Buffer::CreateInfo stagingBufferCreateInfo = {
            .size = sizeof(uint32_t) * 6 * maxFontQuads, .usage = BufferUsage::TransferSrc, .memoryUsage = MemoryUsage::GPUandCPU};
        std::unique_ptr<Buffer> stagingBuffer = Application::getRenderer()->createBuffer(stagingBufferCreateInfo);

        std::vector<uint32_t> indices(maxFontQuads * 6);
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
        colorAttachment.clearColor = Math::Vec4(0.0f);
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
        fontResourceSet = Application::getRenderer()->createResourceSet(fontResourceLayout.get());
        fontResourceSet->update(resourceSetBindings);

        Pipeline::CreateInfo pipelineCreateInfo = {.vertexShaderPath = "Assets/Shaders/BuiltIn.UI.Font.vert",
                                                   .fragmentShaderPath = "Assets/Shaders/BuiltIn.UI.Font.frag",
                                                   .vertexBindings = vertexBindings,
                                                   .vertexAttributes = vertexAttributes,
                                                   .topology = PrimitiveTopology::TriangleList,
                                                   .polygonMode = PolygonMode::Fill,
                                                   .cullMode = CullMode::None,
                                                   .frontFaceClockwise = true,
                                                   .enableBlending = true,
                                                   .enableDepthTest = false,
                                                   .enableDepthWrite = false,
                                                   .colorAttachmentFormats = {Application::getRenderer()->getCurrentRenderTarget()->getFormat()},
                                                   .depthAttachmentFormat = Format::Undefined,
                                                   .resourceLayouts = {fontResourceLayout.get()}};
        fontPipeline = Application::getRenderer()->createPipeline(pipelineCreateInfo);
    }

    bool UI::button(const std::string &label, Math::Vec2 pos, Math::Vec2 size) {
        if (instance->baseVertices.size() / 4 >= maxButtonQuads) {
            AX_CORE_LOG_WARN("UI Quad limit reached! Cannot draw button: {0}", label);
            return false;
        }

        uint32_t id = std::hash<std::string>{}(label);
        bool isClicked = false;

        bool isHovering = instance->mousePosition.x() >= pos.x() && instance->mousePosition.x() <= pos.x() + size.x() &&
                          instance->mousePosition.y() >= pos.y() && instance->mousePosition.y() <= pos.y() + size.y();
        if (isHovering) {
            instance->hotItem = id;
            if (instance->activeItem == 0 && instance->isMouseButtonOneDown) {
                instance->activeItem = id;
            }
        }

        Math::Vec4 color = {0.4f, 0.4f, 0.4f, 1.0f}; // Default: Dark Gray

        if (instance->hotItem == id) {
            if (instance->activeItem == id) {
                color = {0.2f, 0.2f, 0.2f, 1.0f}; // Active: Darker Gray
            } else {
                color = {0.6f, 0.6f, 0.6f, 1.0f}; // Hover: Lighter Gray
            }
        }

        if (instance->activeItem == id && instance->hotItem == id && !instance->isMouseButtonOneDown) {
            isClicked = true;
        }

        instance->addBaseQuad(pos, size, color);

        return isClicked;
    }

    void UI::text(const std::string &text, Math::Vec2 pos, Math::Vec4 color, uint16_t size) {
        auto &atlas = instance->jetBrainsMonoFont.getAsciiAtlas();

        float cursorX = pos.x();
        float cursorY = pos.y();

        uint32_t dpi = Application::getWindow()->getWindowDPI();

        float pixelSize = size * (dpi / 72.0f);
        float fontScale = pixelSize / atlas.unitsPerEm;
        float lineSpacing = pixelSize * 1.2f;

        for (char c : text) {
            if (c == '\n') {
                cursorX = pos.x();
                cursorY += lineSpacing;
                continue;
            }

            auto glyphIt = atlas.glyphs.find(static_cast<uint32_t>(c));
            if (glyphIt == atlas.glyphs.end()) {
                continue;
            }

            const GlyphMetrics &metrics = glyphIt->second;

            float minX = cursorX + (metrics.quadMin.x() * fontScale);
            float minY = cursorY - (metrics.quadMax.y() * fontScale);
            float maxX = cursorX + (metrics.quadMax.x() * fontScale);
            float maxY = cursorY - (metrics.quadMin.y() * fontScale);

            instance->addFontQuad(Math::Vec2(minX, minY), Math::Vec2(maxX - minX, maxY - minY), metrics.uv0, metrics.uv1, color);

            cursorX += (metrics.advance * fontScale);
        }
    }
} // namespace Axiom
