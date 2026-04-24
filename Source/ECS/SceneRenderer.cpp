#include "SceneRenderer.h"
#include "Core/Application.h"

namespace Axiom {
    SceneRenderer::SceneRenderer() {
        createGeometryPassResources();
        createGizmoPassResources();
    }

    void SceneRenderer::geometryPass(const SceneRenderPassData& data) {
        std::vector<SpriteInstance> instances;
        instances.reserve(MAX_SPRITE_QUADS);
        std::vector<Texture*> textureSlots = {Application::getRenderer()->getDefaultTexture()};
        auto entities = data.scene->view<TransformComponent, Sprite2DComponent>();

        for (uint32_t entityId : entities) {
            auto& transform = data.scene->getEntity(entityId).getComponent<TransformComponent>();
            auto& sprite = data.scene->getEntity(entityId).getComponent<Sprite2DComponent>();
            auto textureAsset = AssetManager::getAsset<TextureAsset>(sprite.textureId);

            int32_t textureSlotIndex = 0;

            if (textureAsset) {
                Texture* rawTexture = textureAsset->getTexture();
                bool found = false;
                for (uint32_t i = 0; i < textureSlots.size(); i++) {
                    if (textureSlots[i] == rawTexture) {
                        textureSlotIndex = (int32_t)i;
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    if (textureSlots.size() >= MAX_TEXTURE_SLOTS) {
                        AX_CORE_LOG_WARN("Maximum texture slots reached! Some sprites may not render correctly.");
                        textureSlotIndex = 0; // Fallback to default texture
                    } else {
                        textureSlotIndex = (int32_t)textureSlots.size();
                        textureSlots.push_back(rawTexture);
                    }
                }
            }

            Math::Mat4 model = Math::Mat4::model(transform.position, transform.rotation, transform.scale);
            instances.push_back({model, sprite.color, Math::Vec4((float)textureSlotIndex, 0.0f, 0.0f, 0.0f)});
        }

        if (instances.empty()) {
            return;
        }

        spriteInstanceBuffer->setData<SpriteInstance>(instances);
        Math::iVec2 renderTargetSize = data.renderTarget->getSize();
        Math::Mat4 viewProjection = data.projection * data.view;
        std::vector<ResourceSet::Binding> resourceSetBindings(2);
        resourceSetBindings[0].binding = 0;
        resourceSetBindings[0].type = ResourceType::Texture;
        resourceSetBindings[0].textures = textureSlots;
        resourceSetBindings[0].maxNumberOfResources = MAX_TEXTURE_SLOTS;
        resourceSetBindings[1].binding = 1;
        resourceSetBindings[1].type = ResourceType::Sampler;
        resourceSetBindings[1].samplers = {Application::getRenderer()->getLinearSampler()};
        spriteResourceSets[1]->update(resourceSetBindings);

        spriteRenderPass.colorAttachments[0].texture = data.renderTarget;
        spriteRenderPass.width = renderTargetSize.x();
        spriteRenderPass.height = renderTargetSize.y();
        spriteRenderPass.depthAttachment.texture = data.depthTarget;

        data.commandBuffer->beginRendering(spriteRenderPass);
        data.commandBuffer->bindPipeline(spritePipeline.get());
        data.commandBuffer->bindPushConstants(&viewProjection, sizeof(viewProjection));
        data.commandBuffer->bindVertexBuffers({spriteVertexBuffer.get()});
        data.commandBuffer->bindIndexBuffer(spriteIndexBuffer.get());
        data.commandBuffer->setViewport(0.0f, 0.0f, renderTargetSize.x(), renderTargetSize.y());
        data.commandBuffer->setScissor(0, 0, renderTargetSize.x(), renderTargetSize.y());
        data.commandBuffer->bindResources({spriteResourceSets[0].get(), spriteResourceSets[1].get()});
        data.commandBuffer->drawIndexed(6, instances.size(), 0, 0, 0);
        data.commandBuffer->endRendering();
    }

    void SceneRenderer::gizmoPass(const SceneRenderPassData& data, const Math::Vec3& gizmoPosition) {
        Math::Mat4 model = Math::Mat4::model(gizmoPosition, Math::Vec3::zero(), Math::Vec3(1.0f));
        Math::Mat4 viewProjection = data.projection * data.view * model;

        Math::iVec2 renderTargetSize = data.renderTarget->getSize();
        gizmoRenderPass.colorAttachments[0].texture = data.renderTarget;
        gizmoRenderPass.width = renderTargetSize.x();
        gizmoRenderPass.height = renderTargetSize.y();
        gizmoRenderPass.depthAttachment.texture = data.depthTarget;

        data.commandBuffer->beginRendering(gizmoRenderPass);
        data.commandBuffer->bindPipeline(gizmoPipeline.get());
        data.commandBuffer->bindPushConstants(&viewProjection, sizeof(viewProjection));
        data.commandBuffer->bindVertexBuffers({gizmoVertexBuffer.get()});
        data.commandBuffer->setViewport(0.0f, 0.0f, renderTargetSize.x(), renderTargetSize.y());
        data.commandBuffer->setScissor(0, 0, renderTargetSize.x(), renderTargetSize.y());
        data.commandBuffer->draw(6, 1, 0);
        data.commandBuffer->endRendering();
    }

    void SceneRenderer::createGeometryPassResources() {
        UUID spriteShaderHandle = AssetManager::loadShader("Assets/Shaders/BuiltIn.Sprite2D.axs");
        spriteShader = AssetManager::getAsset<ShaderAsset>(spriteShaderHandle);

        Buffer::CreateInfo vertexBufferCreateInfo = {
            .size = sizeof(SpriteVertex) * 4, .usage = BufferUsage::Vertex | BufferUsage::TransferDst, .memoryUsage = MemoryUsage::GPUOnly};
        spriteVertexBuffer = Application::getRenderer()->createBuffer(vertexBufferCreateInfo);
        Buffer::CreateInfo vertexStagingBufferCreateInfo = {
            .size = sizeof(SpriteVertex) * 4, .usage = BufferUsage::TransferSrc, .memoryUsage = MemoryUsage::GPUandCPU};
        std::unique_ptr<Buffer> vertexStagingBuffer = Application::getRenderer()->createBuffer(vertexStagingBufferCreateInfo);
        std::vector<SpriteVertex> vertices = {
            {{-0.5f, -0.5f}, {0.0f, 0.0f}}, {{0.5f, -0.5f}, {1.0f, 0.0f}}, {{0.5f, 0.5f}, {1.0f, 1.0f}}, {{-0.5f, 0.5f}, {0.0f, 1.0f}}};
        vertexStagingBuffer->setData<SpriteVertex>(vertices);
        auto commandBuffer = Application::getRenderer()->beginSingleTimeCommands();
        commandBuffer->copyBuffer(vertexStagingBuffer.get(), spriteVertexBuffer.get(), vertexBufferCreateInfo.size);
        Application::getRenderer()->endSingleTimeCommands(commandBuffer.get());

        Buffer::CreateInfo indexBufferCreateInfo = {
            .size = sizeof(uint32_t) * 6, .usage = BufferUsage::Index | BufferUsage::TransferDst, .memoryUsage = MemoryUsage::GPUOnly};
        spriteIndexBuffer = Application::getRenderer()->createBuffer(indexBufferCreateInfo);
        Buffer::CreateInfo stagingBufferCreateInfo = {.size = sizeof(uint32_t) * 6, .usage = BufferUsage::TransferSrc, .memoryUsage = MemoryUsage::GPUandCPU};
        std::unique_ptr<Buffer> stagingBuffer = Application::getRenderer()->createBuffer(stagingBufferCreateInfo);
        std::vector<uint32_t> indices = {0, 1, 2, 0, 2, 3};
        commandBuffer = Application::getRenderer()->beginSingleTimeCommands();
        stagingBuffer->setData<uint32_t>(indices);
        commandBuffer->copyBuffer(stagingBuffer.get(), spriteIndexBuffer.get(), stagingBufferCreateInfo.size);
        Application::getRenderer()->endSingleTimeCommands(commandBuffer.get());

        Buffer::CreateInfo instanceBufferCreateInfo = {
            .size = sizeof(SpriteInstance) * MAX_SPRITE_QUADS, .usage = BufferUsage::Storage, .memoryUsage = MemoryUsage::GPUandCPU};
        spriteInstanceBuffer = Application::getRenderer()->createBuffer(instanceBufferCreateInfo);

        RenderAttachment colorAttachment{};
        colorAttachment.loadOp = LoadOp::Clear;
        colorAttachment.storeOp = StoreOp::Store;
        colorAttachment.clearColor = Color::white();
        spriteRenderPass.colorAttachments[0] = colorAttachment;
        spriteRenderPass.colorAttachmentCount = 1;
        RenderAttachment depthAttachment{};
        depthAttachment.loadOp = LoadOp::Clear;
        depthAttachment.storeOp = StoreOp::Store;
        depthAttachment.clearDepth = 1.0f;
        spriteRenderPass.depthAttachment = depthAttachment;
        spriteRenderPass.hasDepthAttachment = true;

        std::vector<VertexBindingDescription> vertexBindings = {{.binding = 0, .stride = sizeof(SpriteVertex), .inputRate = VertexInputRate::Vertex}};
        std::vector<VertexAttributeDescription> vertexAttributes = {
            {.location = 0, .binding = 0, .format = Format::R32G32Sfloat, .offset = offsetof(SpriteVertex, position)},
            {.location = 1, .binding = 0, .format = Format::R32G32Sfloat, .offset = offsetof(SpriteVertex, uv)},
        };

        std::vector<ResourceLayout::BindingCreateInfo> resourceLayoutBindings(1);
        resourceLayoutBindings[0].binding = 0;
        resourceLayoutBindings[0].type = ResourceType::StorageBuffer;
        resourceLayoutBindings[0].stages = ShaderStage::Vertex;
        resourceLayoutBindings[0].count = 1;
        spriteResourceLayouts.push_back(Application::getRenderer()->createResourceLayout(resourceLayoutBindings));
        resourceLayoutBindings.resize(2);
        resourceLayoutBindings[0].binding = 0;
        resourceLayoutBindings[0].type = ResourceType::Texture;
        resourceLayoutBindings[0].stages = ShaderStage::Fragment;
        resourceLayoutBindings[0].count = MAX_TEXTURE_SLOTS;
        resourceLayoutBindings[1].binding = 1;
        resourceLayoutBindings[1].type = ResourceType::Sampler;
        resourceLayoutBindings[1].stages = ShaderStage::Fragment;
        resourceLayoutBindings[1].count = 1;
        spriteResourceLayouts.push_back(Application::getRenderer()->createResourceLayout(resourceLayoutBindings));

        std::vector<ResourceSet::Binding> resourceSetBindings(1);
        resourceSetBindings[0].binding = 0;
        resourceSetBindings[0].type = ResourceType::StorageBuffer;
        resourceSetBindings[0].buffers = {spriteInstanceBuffer.get()};

        Pipeline::CreateInfo pipelineCreateInfo = {.shader = spriteShader->getShader(),
                                                   .vertexBindings = vertexBindings,
                                                   .vertexAttributes = vertexAttributes,
                                                   .topology = PrimitiveTopology::TriangleList,
                                                   .polygonMode = PolygonMode::Fill,
                                                   .cullMode = CullMode::None,
                                                   .frontFaceClockwise = true,
                                                   .enableBlending = false,
                                                   .enableDepthTest = true,
                                                   .enableDepthWrite = true,
                                                   .colorAttachmentFormats = {Application::getRenderer()->getRenderTargetFormat()},
                                                   .depthAttachmentFormat = Application::getRenderer()->getDepthTextureFormat(),
                                                   .resourceLayouts = {spriteResourceLayouts[0].get(), spriteResourceLayouts[1].get()}};
        spritePipeline = Application::getRenderer()->createPipeline(pipelineCreateInfo);
        spriteResourceSets.push_back(spritePipeline->createResourceSet(spriteResourceLayouts[0].get()));
        spriteResourceSets.push_back(spritePipeline->createResourceSet(spriteResourceLayouts[1].get()));
        spriteResourceSets[0]->update(resourceSetBindings);
    }

    void SceneRenderer::createGizmoPassResources() {
        UUID gizmoShaderHandle = AssetManager::loadShader("Assets/Shaders/BuiltIn.UI.Gizmo.axs");
        gizmoShader = AssetManager::getAsset<ShaderAsset>(gizmoShaderHandle);

        Buffer::CreateInfo vertexBufferCreateInfo = {.size = sizeof(GizmoVertex) * 2 * 3, .usage = BufferUsage::Vertex, .memoryUsage = MemoryUsage::GPUandCPU};
        gizmoVertexBuffer = Application::getRenderer()->createBuffer(vertexBufferCreateInfo);
        Buffer::CreateInfo stagingBufferCreateInfo = {
            .size = sizeof(GizmoVertex) * 2 * 3, .usage = BufferUsage::TransferSrc, .memoryUsage = MemoryUsage::GPUandCPU};
        std::unique_ptr<Buffer> stagingBuffer = Application::getRenderer()->createBuffer(stagingBufferCreateInfo);
        std::vector<GizmoVertex> vertices = {
            {{0.0f, 0.0f, 0.0f, 1.0f}, Color::red()},     {{100.0f, 0.0f, 0.0f, 1.0f}, Color::red()}, {{0.0f, 0.0f, 0.0f, 1.0f}, Color::green()},
            {{0.0f, 100.0f, 0.0f, 1.0f}, Color::green()}, {{0.0f, 0.0f, 0.0f, 1.0f}, Color::blue()},  {{0.0f, 0.0f, 100.0f, 1.0f}, Color::blue()},
        };
        stagingBuffer->setData<GizmoVertex>(vertices);
        auto commandBuffer = Application::getRenderer()->beginSingleTimeCommands();
        commandBuffer->copyBuffer(stagingBuffer.get(), gizmoVertexBuffer.get(), vertexBufferCreateInfo.size);
        Application::getRenderer()->endSingleTimeCommands(commandBuffer.get());

        RenderAttachment colorAttachment{};
        colorAttachment.loadOp = LoadOp::Load;
        colorAttachment.storeOp = StoreOp::Store;
        colorAttachment.clearColor = Color::transparent();
        gizmoRenderPass.colorAttachments[0] = colorAttachment;
        gizmoRenderPass.colorAttachmentCount = 1;

        std::vector<VertexBindingDescription> vertexBindings = {{.binding = 0, .stride = sizeof(GizmoVertex), .inputRate = VertexInputRate::Vertex}};
        std::vector<VertexAttributeDescription> vertexAttributes = {
            {.location = 0, .binding = 0, .format = Format::R32G32B32Sfloat, .offset = offsetof(GizmoVertex, position)},
            {.location = 1, .binding = 0, .format = Format::R32G32B32A32Sfloat, .offset = offsetof(GizmoVertex, color)},
        };

        Pipeline::CreateInfo pipelineCreateInfo = {.shader = gizmoShader->getShader(),
                                                   .vertexBindings = vertexBindings,
                                                   .vertexAttributes = vertexAttributes,
                                                   .topology = PrimitiveTopology::LineList,
                                                   .polygonMode = PolygonMode::Fill,
                                                   .cullMode = CullMode::None,
                                                   .frontFaceClockwise = true,
                                                   .enableBlending = true,
                                                   .enableDepthTest = false,
                                                   .enableDepthWrite = false,
                                                   .colorAttachmentFormats = {Application::getRenderer()->getRenderTargetFormat()},
                                                   .depthAttachmentFormat = Format::Undefined,
                                                   .resourceLayouts = {}};
        gizmoPipeline = Application::getRenderer()->createPipeline(pipelineCreateInfo);
    }
} // namespace Axiom