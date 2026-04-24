#include "SceneRenderer.h"
#include "Core/Application.h"

namespace Axiom {
    SceneRenderer::SceneRenderer() {
        createGeometryPassResources();
        createGizmoPassResources();
    }

    void SceneRenderer::geometryPass(const SceneRenderPassData& data) {
        std::vector<SpriteInstance> spriteInstances;
        spriteInstances.reserve(MAX_SPRITE_INSTANCES);
        std::vector<Texture*> spriteTextureSlots = {Application::getRenderer()->getDefaultTexture()};
        auto spriteEntities = data.scene->view<TransformComponent, Sprite2DComponent>();

        for (uint32_t entityId : spriteEntities) {
            auto& transform = data.scene->getEntity(entityId).getComponent<TransformComponent>();
            auto& sprite = data.scene->getEntity(entityId).getComponent<Sprite2DComponent>();
            auto textureAsset = AssetManager::getAsset<TextureAsset>(sprite.textureId);

            int32_t textureSlotIndex = 0;

            if (textureAsset) {
                Texture* rawTexture = textureAsset->getTexture();
                bool found = false;
                for (uint32_t i = 0; i < spriteTextureSlots.size(); i++) {
                    if (spriteTextureSlots[i] == rawTexture) {
                        textureSlotIndex = (int32_t)i;
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    if (spriteTextureSlots.size() >= MAX_TEXTURE_SLOTS) {
                        AX_CORE_LOG_WARN("Maximum texture slots reached! Some sprites may not render correctly.");
                        textureSlotIndex = 0; // Fallback to default texture
                    } else {
                        textureSlotIndex = (int32_t)spriteTextureSlots.size();
                        spriteTextureSlots.push_back(rawTexture);
                    }
                }
            }

            Math::Mat4 model = Math::Mat4::model(transform.position, transform.rotation, transform.scale);
            spriteInstances.push_back({model, sprite.color, Math::Vec4((float)textureSlotIndex, 0.0f, 0.0f, 0.0f)});
        }

        std::unordered_map<UUID, std::vector<MeshInstance>> meshBatches;
        auto meshEntities = data.scene->view<TransformComponent, MeshComponent>();

        for (uint32_t entityId : meshEntities) {
            auto& transform = data.scene->getEntity(entityId).getComponent<TransformComponent>();
            auto& mesh = data.scene->getEntity(entityId).getComponent<MeshComponent>();
            auto model = Math::Mat4::model(transform.position, transform.rotation, transform.scale);
            meshBatches[mesh.meshId].push_back({model});
        }

        Math::iVec2 renderTargetSize = data.renderTarget->getSize();
        Math::Mat4 viewProjection = data.projection * data.view;
        geometryRenderPass.colorAttachments[0].texture = data.renderTarget;
        geometryRenderPass.width = renderTargetSize.x();
        geometryRenderPass.height = renderTargetSize.y();
        geometryRenderPass.depthAttachment.texture = data.depthTarget;
        data.commandBuffer->beginRendering(geometryRenderPass);

        if (!spriteInstances.empty()) {
            spriteInstanceBuffer->setData<SpriteInstance>(spriteInstances);
            std::vector<ResourceSet::Binding> resourceSetBindings(2);
            resourceSetBindings[0].binding = 0;
            resourceSetBindings[0].type = ResourceType::Texture;
            resourceSetBindings[0].textures = spriteTextureSlots;
            resourceSetBindings[0].maxNumberOfResources = MAX_TEXTURE_SLOTS;
            resourceSetBindings[1].binding = 1;
            resourceSetBindings[1].type = ResourceType::Sampler;
            resourceSetBindings[1].samplers = {Application::getRenderer()->getLinearSampler()};
            spriteResourceSets[1]->update(resourceSetBindings);

            data.commandBuffer->bindPipeline(spritePipeline.get());
            data.commandBuffer->bindPushConstants(&viewProjection, sizeof(viewProjection));
            data.commandBuffer->bindVertexBuffers({spriteVertexBuffer.get()});
            data.commandBuffer->bindIndexBuffer(spriteIndexBuffer.get());
            data.commandBuffer->setViewport(0.0f, 0.0f, renderTargetSize.x(), renderTargetSize.y());
            data.commandBuffer->setScissor(0, 0, renderTargetSize.x(), renderTargetSize.y());
            data.commandBuffer->bindResources({spriteResourceSets[0].get(), spriteResourceSets[1].get()});
            data.commandBuffer->drawIndexed(6, spriteInstances.size(), 0, 0, 0);
        }

        if (!meshBatches.empty()) {
            data.commandBuffer->bindPipeline(meshPipeline.get());
            data.commandBuffer->bindPushConstants(&viewProjection, sizeof(viewProjection));
            data.commandBuffer->setViewport(0.0f, 0.0f, renderTargetSize.x(), renderTargetSize.y());
            data.commandBuffer->setScissor(0, 0, renderTargetSize.x(), renderTargetSize.y());
            data.commandBuffer->bindVertexBuffers({AssetManager::getGlobalVertexBuffer()});
            data.commandBuffer->bindIndexBuffer(AssetManager::getGlobalIndexBuffer());

            for (const auto& [meshId, meshInstances] : meshBatches) {
                auto meshAsset = AssetManager::getAsset<MeshAsset>(meshId);
                if (!meshAsset) {
                    AX_CORE_LOG_WARN("Mesh asset with ID {} not found! Skipping mesh.", meshId);
                    continue;
                }

                meshInstanceBuffer->setData<MeshInstance>(meshInstances);

                data.commandBuffer->bindResources({meshResourceSets[0].get()});
                data.commandBuffer->drawIndexed(meshAsset->getIndexCount(), meshInstances.size(), meshAsset->getIndexOffset(), meshAsset->getVertexOffset(), 0);
            }
        }

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
        RenderAttachment colorAttachment{};
        colorAttachment.loadOp = LoadOp::Clear;
        colorAttachment.storeOp = StoreOp::Store;
        colorAttachment.clearColor = Color::white();
        geometryRenderPass.colorAttachments[0] = colorAttachment;
        geometryRenderPass.colorAttachmentCount = 1;
        RenderAttachment depthAttachment{};
        depthAttachment.loadOp = LoadOp::Clear;
        depthAttachment.storeOp = StoreOp::Store;
        depthAttachment.clearDepth = 1.0f;
        geometryRenderPass.depthAttachment = depthAttachment;
        geometryRenderPass.hasDepthAttachment = true;

        // 2d objects
        UUID spriteShaderHandle = AssetManager::loadShader("Assets/Shaders/BuiltIn.Sprite2D.axs");
        spriteShader = AssetManager::getAsset<ShaderAsset>(spriteShaderHandle);

        Buffer::CreateInfo spriteVertexBufferCreateInfo = {
            .size = sizeof(SpriteVertex) * 4, .usage = BufferUsage::Vertex | BufferUsage::TransferDst, .memoryUsage = MemoryUsage::GPUOnly};
        spriteVertexBuffer = Application::getRenderer()->createBuffer(spriteVertexBufferCreateInfo);
        Buffer::CreateInfo spriteVertexStagingBufferCreateInfo = {
            .size = sizeof(SpriteVertex) * 4, .usage = BufferUsage::TransferSrc, .memoryUsage = MemoryUsage::GPUandCPU};
        std::unique_ptr<Buffer> spriteVertexStagingBuffer = Application::getRenderer()->createBuffer(spriteVertexStagingBufferCreateInfo);
        std::vector<SpriteVertex> vertices = {
            {{-0.5f, -0.5f}, {0.0f, 0.0f}}, {{0.5f, -0.5f}, {1.0f, 0.0f}}, {{0.5f, 0.5f}, {1.0f, 1.0f}}, {{-0.5f, 0.5f}, {0.0f, 1.0f}}};
        spriteVertexStagingBuffer->setData<SpriteVertex>(vertices);
        auto commandBuffer = Application::getRenderer()->beginSingleTimeCommands();
        commandBuffer->copyBuffer(spriteVertexStagingBuffer.get(), spriteVertexBuffer.get(), spriteVertexBufferCreateInfo.size);
        Application::getRenderer()->endSingleTimeCommands(commandBuffer.get());

        Buffer::CreateInfo spriteIndexBufferCreateInfo = {
            .size = sizeof(uint32_t) * 6, .usage = BufferUsage::Index | BufferUsage::TransferDst, .memoryUsage = MemoryUsage::GPUOnly};
        spriteIndexBuffer = Application::getRenderer()->createBuffer(spriteIndexBufferCreateInfo);
        Buffer::CreateInfo spriteStagingBufferCreateInfo = {
            .size = sizeof(uint32_t) * 6, .usage = BufferUsage::TransferSrc, .memoryUsage = MemoryUsage::GPUandCPU};
        std::unique_ptr<Buffer> spriteStagingBuffer = Application::getRenderer()->createBuffer(spriteStagingBufferCreateInfo);
        std::vector<uint32_t> indices = {0, 1, 2, 0, 2, 3};
        commandBuffer = Application::getRenderer()->beginSingleTimeCommands();
        spriteStagingBuffer->setData<uint32_t>(indices);
        commandBuffer->copyBuffer(spriteStagingBuffer.get(), spriteIndexBuffer.get(), spriteStagingBufferCreateInfo.size);
        Application::getRenderer()->endSingleTimeCommands(commandBuffer.get());

        Buffer::CreateInfo spriteInstanceBufferCreateInfo = {
            .size = sizeof(SpriteInstance) * MAX_SPRITE_INSTANCES, .usage = BufferUsage::Storage, .memoryUsage = MemoryUsage::GPUandCPU};
        spriteInstanceBuffer = Application::getRenderer()->createBuffer(spriteInstanceBufferCreateInfo);

        std::vector<VertexBindingDescription> spriteVertexBindings = {{.binding = 0, .stride = sizeof(SpriteVertex), .inputRate = VertexInputRate::Vertex}};
        std::vector<VertexAttributeDescription> spriteVertexAttributes = {
            {.location = 0, .binding = 0, .format = Format::R32G32Sfloat, .offset = offsetof(SpriteVertex, position)},
            {.location = 1, .binding = 0, .format = Format::R32G32Sfloat, .offset = offsetof(SpriteVertex, uv)},
        };

        std::vector<ResourceLayout::BindingCreateInfo> spriteResourceLayoutBindings(1);
        spriteResourceLayoutBindings[0].binding = 0;
        spriteResourceLayoutBindings[0].type = ResourceType::StorageBuffer;
        spriteResourceLayoutBindings[0].stages = ShaderStage::Vertex;
        spriteResourceLayoutBindings[0].count = 1;
        spriteResourceLayouts.push_back(Application::getRenderer()->createResourceLayout(spriteResourceLayoutBindings));
        std::vector<ResourceLayout::BindingCreateInfo> spriteResourceLayoutBindings2(2);
        spriteResourceLayoutBindings2[0].binding = 0;
        spriteResourceLayoutBindings2[0].type = ResourceType::Texture;
        spriteResourceLayoutBindings2[0].stages = ShaderStage::Fragment;
        spriteResourceLayoutBindings2[0].count = MAX_TEXTURE_SLOTS;
        spriteResourceLayoutBindings2[1].binding = 1;
        spriteResourceLayoutBindings2[1].type = ResourceType::Sampler;
        spriteResourceLayoutBindings2[1].stages = ShaderStage::Fragment;
        spriteResourceLayoutBindings2[1].count = 1;
        spriteResourceLayouts.push_back(Application::getRenderer()->createResourceLayout(spriteResourceLayoutBindings2));

        std::vector<ResourceSet::Binding> spriteResourceSetBindings(1);
        spriteResourceSetBindings[0].binding = 0;
        spriteResourceSetBindings[0].type = ResourceType::StorageBuffer;
        spriteResourceSetBindings[0].buffers = {spriteInstanceBuffer.get()};

        Pipeline::CreateInfo spritePipelineCreateInfo = {.shader = spriteShader->getShader(),
                                                         .vertexBindings = spriteVertexBindings,
                                                         .vertexAttributes = spriteVertexAttributes,
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
        spritePipeline = Application::getRenderer()->createPipeline(spritePipelineCreateInfo);
        spriteResourceSets.push_back(spritePipeline->createResourceSet(spriteResourceLayouts[0].get()));
        spriteResourceSets.push_back(spritePipeline->createResourceSet(spriteResourceLayouts[1].get()));
        spriteResourceSets[0]->update(spriteResourceSetBindings);

        // 3d objects
        UUID meshShaderHandle = AssetManager::loadShader("Assets/Shaders/BuiltIn.Mesh.axs");
        meshShader = AssetManager::getAsset<ShaderAsset>(meshShaderHandle);

        Buffer::CreateInfo meshInstanceBufferCreateInfo = {
            .size = sizeof(MeshInstance) * MAX_MESH_INSTANCES, .usage = BufferUsage::Storage, .memoryUsage = MemoryUsage::GPUandCPU};
        meshInstanceBuffer = Application::getRenderer()->createBuffer(meshInstanceBufferCreateInfo);

        std::vector<VertexBindingDescription> meshVertexBindings = {{.binding = 0, .stride = sizeof(MeshVertex), .inputRate = VertexInputRate::Vertex}};
        std::vector<VertexAttributeDescription> meshVertexAttributes = {
            {.location = 0, .binding = 0, .format = Format::R32G32B32Sfloat, .offset = offsetof(MeshVertex, position)},
            {.location = 1, .binding = 0, .format = Format::R32G32B32Sfloat, .offset = offsetof(MeshVertex, normal)},
            {.location = 2, .binding = 0, .format = Format::R32G32Sfloat, .offset = offsetof(MeshVertex, uv)},
        };

        std::vector<ResourceLayout::BindingCreateInfo> meshResourceLayoutBindings(1);
        meshResourceLayoutBindings[0].binding = 0;
        meshResourceLayoutBindings[0].type = ResourceType::StorageBuffer;
        meshResourceLayoutBindings[0].stages = ShaderStage::Vertex;
        meshResourceLayoutBindings[0].count = 1;
        meshResourceLayouts.push_back(Application::getRenderer()->createResourceLayout(meshResourceLayoutBindings));

        std::vector<ResourceSet::Binding> meshResourceSetBindings(1);
        meshResourceSetBindings[0].binding = 0;
        meshResourceSetBindings[0].type = ResourceType::StorageBuffer;
        meshResourceSetBindings[0].buffers = {meshInstanceBuffer.get()};

        Pipeline::CreateInfo meshPipelineCreateInfo = {.shader = meshShader->getShader(),
                                                       .vertexBindings = meshVertexBindings,
                                                       .vertexAttributes = meshVertexAttributes,
                                                       .topology = PrimitiveTopology::TriangleList,
                                                       .polygonMode = PolygonMode::Fill,
                                                       .cullMode = CullMode::Back,
                                                       .frontFaceClockwise = true,
                                                       .enableBlending = false,
                                                       .enableDepthTest = true,
                                                       .enableDepthWrite = true,
                                                       .colorAttachmentFormats = {Application::getRenderer()->getRenderTargetFormat()},
                                                       .depthAttachmentFormat = Application::getRenderer()->getDepthTextureFormat(),
                                                       .resourceLayouts = {meshResourceLayouts.back().get()}};
        meshPipeline = Application::getRenderer()->createPipeline(meshPipelineCreateInfo);
        meshResourceSets.push_back(meshPipeline->createResourceSet(meshResourceLayouts.back().get()));
        meshResourceSets.back()->update(meshResourceSetBindings);
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