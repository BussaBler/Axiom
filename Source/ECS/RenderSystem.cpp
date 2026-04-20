#include "RenderSystem.h"
#include "Core/Application.h"

namespace Axiom {
    RenderSystem::RenderSystem() {
        createSpriteRenderObjects();
    }

    void RenderSystem::onUpdate(Registry* registry, float deltaTime) {
    }

    void RenderSystem::onRender(Registry* registry, CommandBuffer* commandBuffer, Texture* renderTarget, const Math::Mat4& projection, const Math::Mat4& view) {
        std::vector<SpriteInstance> instances;
        instances.reserve(MAX_SPRITE_QUADS);
        std::vector<Texture*> textureSlots = {Application::getRenderer()->getDefaultTexture()};
        auto entities = registry->view<TransformComponent, Sprite2DComponent>();

        for (uint32_t entityId : entities) {
            auto& transform = registry->getComponent<TransformComponent>(entityId);
            auto& sprite = registry->getComponent<Sprite2DComponent>(entityId);
            auto textureAsset = AssetManager::getAsset<TextureAsset>(sprite.textureId);

            int32_t textureSlotIndex = 0;

            if (textureAsset) {
                Texture* rawTexture = textureAsset->getTexture().get();
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
        Math::iVec2 renderTargetSize = renderTarget->getSize();
        Math::Mat4 viewProjection = projection * view;
        std::vector<ResourceSet::Binding> resourceSetBindings(2);
        resourceSetBindings[0].binding = 0;
        resourceSetBindings[0].type = ResourceType::Texture;
        resourceSetBindings[0].textures = textureSlots;
        resourceSetBindings[0].maxNumberOfResources = MAX_TEXTURE_SLOTS;
        resourceSetBindings[1].binding = 1;
        resourceSetBindings[1].type = ResourceType::Sampler;
        resourceSetBindings[1].samplers = {Application::getRenderer()->getLinearSampler()};
        spriteResourceSets[1]->update(resourceSetBindings);

        spriteRenderPass.colorAttachments[0].texture = renderTarget;
        spriteRenderPass.width = renderTargetSize.x();
        spriteRenderPass.height = renderTargetSize.y();

        commandBuffer->beginRendering(spriteRenderPass);
        commandBuffer->bindPipeline(spritePipeline.get());
        commandBuffer->bindPushConstants(&viewProjection, sizeof(viewProjection));
        commandBuffer->bindVertexBuffers({spriteVertexBuffer.get()});
        commandBuffer->bindIndexBuffer(spriteIndexBuffer.get());
        commandBuffer->setViewport(0.0f, 0.0f, renderTargetSize.x(), renderTargetSize.y());
        commandBuffer->setScissor(0, 0, renderTargetSize.x(), renderTargetSize.y());
        commandBuffer->bindResources({spriteResourceSets[0].get(), spriteResourceSets[1].get()});
        commandBuffer->drawIndexed(6, instances.size(), 0, 0, 0);
        commandBuffer->endRendering();
    }

    void RenderSystem::createSpriteRenderObjects() {
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

        std::vector<VertexBindingDescription> vertexBindings = {{.binding = 0, .stride = sizeof(SpriteVertex), .inputRate = VertexInputRate::Vertex}};
        std::vector<VertexAttributeDescription> vertexAttributes = {
            {.location = 0, .binding = 0, .format = Format::R32G32Sfloat, .offset = offsetof(SpriteVertex, pos)},
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
                                                   .enableBlending = true,
                                                   .enableDepthTest = false,
                                                   .enableDepthWrite = false,
                                                   .colorAttachmentFormats = {Application::getRenderer()->getRenderTargetFormat()},
                                                   .depthAttachmentFormat = Format::Undefined,
                                                   .resourceLayouts = {spriteResourceLayouts[0].get(), spriteResourceLayouts[1].get()}};
        spritePipeline = Application::getRenderer()->createPipeline(pipelineCreateInfo);
        spriteResourceSets.push_back(spritePipeline->createResourceSet(spriteResourceLayouts[0].get()));
        spriteResourceSets.push_back(spritePipeline->createResourceSet(spriteResourceLayouts[1].get()));
        spriteResourceSets[0]->update(resourceSetBindings);
    }
} // namespace Axiom
