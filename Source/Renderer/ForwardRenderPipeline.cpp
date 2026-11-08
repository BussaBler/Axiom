#include "axpch.h"

#include "ForwardRenderPipeline.h"

#include "Asset/MeshAsset.h"
#include "Asset/ShaderAsset.h"
#include "Asset/UUID.h"
#include "Core/Assert.h"
#include "ECS/Components/MeshComponent.h"
#include "ECS/Components/Sprite2DComponent.h"
#include "ECS/Components/TransformComponent.h"
#include "Math/Color.h"
#include "Math/Mat.h"
#include "Math/Vec.h"
#include "Renderer/Buffer.h"
#include "Renderer/CommandBuffer.h"
#include "Renderer/Pipeline.h"
#include "Renderer/RenderFeature.h"
#include "Renderer/RenderGraph.h"
#include "Renderer/RenderPass.h"
#include "Renderer/RenderPipeline.h"
#include "Renderer/ResourceLayout.h"
#include "Renderer/ResourceSet.h"
#include "Renderer/Texture.h"
#include "Renderer/TextureRegistry.h"
#include "Scene/Scene.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <vector>

namespace Axiom {
    ForwardRenderPipeline::ForwardRenderPipeline(RenderHardwareInterface& rhi, AssetManager& assetManager, TextureRegistry& textureRegistry,
                                                 PSOManager& psoManager)
        : rhi(rhi), assetManager(assetManager), textureRegistry(textureRegistry), psoManager(psoManager) {
        createGlobalData();
        createOpaquePassData();
        createSkyboxPassData();
        createSpritePassData();
        createWorldGridPassData();
        createGizmosPassData();
    }

    ForwardRenderPipeline::~ForwardRenderPipeline() {
    }

    void ForwardRenderPipeline::beginFrame() {
        globalDataAllocations = 0;
        totalInstanceCount = 0;
    }

    void ForwardRenderPipeline::buildRenderGraph(RenderGraph& renderGraph, const RenderContext& context) {
        AX_CORE_ASSERT(globalDataAllocations < MAX_VIEWPORTS, "Exceeded maximum viewports per frame");

        uint32_t currentViewportIndex = globalDataAllocations++;
        currentFrameIndex = rhi.getCurrentFrameIndex();

        globalData = {.viewMatrix = context.viewMatrix, .projectionMatrix = context.projectionMatrix, .cameraPosition = context.cameraPosition};
        globalDataBuffers[currentFrameIndex]->setData(&globalData, sizeof(GlobalData), currentViewportIndex * sizeof(GlobalData));

        RGTextureHandle colorBufferHandle = renderGraph.importTexture("ColorBuffer", context.renderTarget);
        RGTextureHandle depthBufferHandle = renderGraph.importTexture("DepthBuffer", context.depthTarget);

        renderGraph.addBlackboardData<GlobalResourceData>({.scene = context.targetScene,
                                                           .colorBuffer = colorBufferHandle,
                                                           .depthBuffer = depthBufferHandle,
                                                           .viewportIndex = currentViewportIndex,
                                                           .gizmoPosition = context.gizmosPosition});

        injectRenderFeatures(RenderInjectionPoint::BeforeOpaque, renderGraph, context);
        renderGraph.addPass<DefaultPassData>("Opaque Pass", [this](PassBuilder& builder, DefaultPassData& passData, Blackboard& blackboard) {
            const auto& resourceData = blackboard.get<GlobalResourceData>();
            builder.writeTexture(resourceData.colorBuffer, TextureState::RenderTarget);
            builder.writeTexture(resourceData.depthBuffer, TextureState::DepthStencilTarget);
            passData.renderTarget = resourceData.colorBuffer;
            passData.depthTarget = resourceData.depthBuffer;
            passData.scene = resourceData.scene;
            passData.viewportIndex = resourceData.viewportIndex;

            return [this](const DefaultPassData& passData, const PassResources& resources, CommandBuffer* cmd, const Blackboard& blackboard,
                          const FrameContext& frameCtx) {
                opaquePass(passData.scene, resources.getTexture(passData.renderTarget), resources.getTexture(passData.depthTarget), cmd,
                           passData.viewportIndex);
            };
        });
        injectRenderFeatures(RenderInjectionPoint::AfterOpaque, renderGraph, context);

        injectRenderFeatures(RenderInjectionPoint::BeforeSkybox, renderGraph, context);
        if (context.shouldDrawSkybox) {
            renderGraph.addPass<DefaultPassData>("Skybox Pass", [this](PassBuilder& builder, DefaultPassData& passData, Blackboard& blackboard) {
                const auto& resourceData = blackboard.get<GlobalResourceData>();
                builder.writeTexture(resourceData.colorBuffer, TextureState::RenderTarget);
                builder.writeTexture(resourceData.depthBuffer, TextureState::DepthStencilTarget);
                passData.renderTarget = resourceData.colorBuffer;
                passData.depthTarget = resourceData.depthBuffer;
                passData.scene = resourceData.scene;

                return [this](const DefaultPassData& passData, const PassResources& resources, CommandBuffer* cmd, const Blackboard& blackboard,
                              const FrameContext& frameCtx) {
                    skyboxPass(passData.scene, resources.getTexture(passData.renderTarget), resources.getTexture(passData.depthTarget), cmd);
                };
            });
        }
        injectRenderFeatures(RenderInjectionPoint::AfterSkybox, renderGraph, context);

        renderGraph.addPass<DefaultPassData>("Sprite Pass", [this](PassBuilder& builder, DefaultPassData& passData, Blackboard& blackboard) {
            const auto& resourceData = blackboard.get<GlobalResourceData>();
            builder.writeTexture(resourceData.colorBuffer, TextureState::RenderTarget);
            builder.writeTexture(resourceData.depthBuffer, TextureState::DepthStencilTarget);
            passData.renderTarget = resourceData.colorBuffer;
            passData.depthTarget = resourceData.depthBuffer;
            passData.scene = resourceData.scene;
            passData.viewportIndex = resourceData.viewportIndex;

            return [this](const DefaultPassData& passData, const PassResources& resources, CommandBuffer* cmd, const Blackboard& blackboard,
                          const FrameContext& frameCtx) {
                spritePass(passData.scene, resources.getTexture(passData.renderTarget), resources.getTexture(passData.depthTarget), cmd,
                           passData.viewportIndex);
            };
        });

        if (context.shouldDrawWorldGrid) {
            renderGraph.addPass<DefaultPassData>("World Grid Pass", [this](PassBuilder& builder, DefaultPassData& passData, Blackboard& blackboard) {
                const auto& resourceData = blackboard.get<GlobalResourceData>();
                builder.writeTexture(resourceData.colorBuffer, TextureState::RenderTarget);
                builder.writeTexture(resourceData.depthBuffer, TextureState::DepthStencilTarget);
                passData.renderTarget = resourceData.colorBuffer;
                passData.depthTarget = resourceData.depthBuffer;
                passData.scene = resourceData.scene;

                return [this](const DefaultPassData& passData, const PassResources& resources, CommandBuffer* cmd, const Blackboard& blackboard,
                              const FrameContext& frameCtx) {
                    worldGridPass(passData.scene, resources.getTexture(passData.renderTarget), resources.getTexture(passData.depthTarget), cmd);
                };
            });
        }
        if (context.shouldDrawGizmos) {
            renderGraph.addPass<GizmosPassData>("Gizmos Pass", [this](PassBuilder& builder, GizmosPassData& passData, Blackboard& blackboard) {
                const auto& resourceData = blackboard.get<GlobalResourceData>();
                builder.writeTexture(resourceData.colorBuffer, TextureState::RenderTarget);
                builder.writeTexture(resourceData.depthBuffer, TextureState::DepthStencilTarget);
                passData.renderTarget = resourceData.colorBuffer;
                passData.depthTarget = resourceData.depthBuffer;
                passData.scene = resourceData.scene;
                passData.gizmosPosition = resourceData.gizmoPosition;

                return [this](const GizmosPassData& passData, const PassResources& resources, CommandBuffer* cmd, const Blackboard& blackboard,
                              const FrameContext& frameCtx) {
                    gizmosPass(passData.scene, resources.getTexture(passData.renderTarget), resources.getTexture(passData.depthTarget), cmd,
                               passData.gizmosPosition);
                };
            });
        }

        renderGraph.addPass<DefaultPassData>("Prepare Present Pass", [](PassBuilder& builder, DefaultPassData& passData, Blackboard& blackboard) {
            const auto& resourceData = blackboard.get<GlobalResourceData>();
            builder.readTexture(resourceData.colorBuffer, TextureState::Present);
            return [](const auto&...) {};
        });
    }

    void ForwardRenderPipeline::createGlobalData() {
        globalData = {};
        std::vector<ResourceLayout::BindingCreateInfo> bindingsCreateInfo(1);
        bindingsCreateInfo[0].binding = Bindings::GlobalDataBinding;
        bindingsCreateInfo[0].type = ResourceType::UniformBuffer;
        bindingsCreateInfo[0].stages = ShaderStage::Vertex | ShaderStage::Fragment;
        bindingsCreateInfo[0].count = 1;
        globalDataResourceLayout = rhi.createResourceLayout(bindingsCreateInfo);

        globalDataResourceSets.resize(rhi.MAX_FRAMES_IN_FLIGHT);
        globalDataBuffers.resize(rhi.MAX_FRAMES_IN_FLIGHT);
        for (uint32_t i = 0; i < rhi.MAX_FRAMES_IN_FLIGHT; i++) {
            globalDataResourceSets[i] = rhi.createResourceSet(globalDataResourceLayout.get());

            Buffer::CreateInfo globalDataBufferCreateInfo = {
                .size = sizeof(GlobalData) * MAX_VIEWPORTS, .usage = BufferUsage::Uniform, .memoryUsage = MemoryUsage::GPUandCPU};
            globalDataBuffers[i] = rhi.createBuffer(globalDataBufferCreateInfo);

            Buffer* bufferData[] = {globalDataBuffers[i].get()};
            std::vector<ResourceSet::Binding> bindingUpdateInfo(1);
            bindingUpdateInfo[0].binding = Bindings::GlobalDataBinding;
            bindingUpdateInfo[0].type = ResourceType::UniformBuffer;
            bindingUpdateInfo[0].stages = ShaderStage::Vertex | ShaderStage::Fragment;
            bindingUpdateInfo[0].data = bufferData;
            globalDataResourceSets[i]->update(bindingUpdateInfo);
        }
    }

    void ForwardRenderPipeline::createOpaquePassData() {
        RenderAttachment colorAttachment = {.loadOp = LoadOp::Clear, .storeOp = StoreOp::Store, .clearColor = Color::transparent()};
        RenderAttachment depthAttachment = {.loadOp = LoadOp::Clear, .storeOp = StoreOp::Store, .clearDepth = 1.0f};
        opaqueRenderPass = {};
        opaqueRenderPass.colorAttachments[0] = colorAttachment;
        opaqueRenderPass.colorAttachmentCount = 1;
        opaqueRenderPass.depthAttachment = depthAttachment;
        opaqueRenderPass.hasDepthAttachment = true;

        std::vector<ResourceLayout::BindingCreateInfo> bindingsCreateInfo(1);
        bindingsCreateInfo[0].binding = 1;
        bindingsCreateInfo[0].type = ResourceType::StorageBuffer;
        bindingsCreateInfo[0].stages = ShaderStage::Vertex;
        bindingsCreateInfo[0].count = 1;
        instanceResourceLayout = rhi.createResourceLayout(bindingsCreateInfo);

        instanceResourceSets.resize(rhi.MAX_FRAMES_IN_FLIGHT);
        instanceSSBOs.resize(rhi.MAX_FRAMES_IN_FLIGHT);
        for (uint32_t i = 0; i < rhi.MAX_FRAMES_IN_FLIGHT; i++) {
            instanceResourceSets[i] = rhi.createResourceSet(instanceResourceLayout.get());

            Buffer::CreateInfo instanceSSBOCreateInfo = {
                .size = sizeof(Math::Mat4) * MAX_INSTANCES_PER_FRAME, .usage = BufferUsage::Storage, .memoryUsage = MemoryUsage::GPUandCPU};
            instanceSSBOs[i] = rhi.createBuffer(instanceSSBOCreateInfo);

            Buffer* bufferData[] = {instanceSSBOs[i].get()};
            std::vector<ResourceSet::Binding> bindingsUpdateInfo(1);
            bindingsUpdateInfo[0].binding = 1;
            bindingsUpdateInfo[0].type = ResourceType::StorageBuffer;
            bindingsUpdateInfo[0].stages = ShaderStage::Vertex;
            bindingsUpdateInfo[0].data = bufferData;
            instanceResourceSets[i]->update(bindingsUpdateInfo);
        }
    }

    void ForwardRenderPipeline::createSpritePassData() {
        RenderAttachment colorAttachment = {.loadOp = LoadOp::Load, .storeOp = StoreOp::Store};
        RenderAttachment depthAttachment = {.loadOp = LoadOp::Load, .storeOp = StoreOp::Store};
        spriteRenderPass = {};
        spriteRenderPass.colorAttachments[0] = colorAttachment;
        spriteRenderPass.colorAttachmentCount = 1;
        spriteRenderPass.depthAttachment = depthAttachment;
        spriteRenderPass.hasDepthAttachment = true;

        struct QuadVertex {
            Math::Vec3 postion;
            Math::Vec2 uv;
        };

        std::vector<QuadVertex> quadVertices = {
            {{-0.5f, -0.5f, 0.0f}, {0.0f, 0.0f}}, {{0.5f, -0.5f, 0.0f}, {1.0f, 0.0f}}, {{0.5f, 0.5f, 0.0f}, {1.0f, 1.0f}}, {{-0.5f, 0.5f, 0.0f}, {0.0f, 1.0f}}};
        std::vector<uint32_t> quadIndices = {0, 1, 2, 2, 3, 0};

        Buffer::CreateInfo spriteVertexBufferCreateInfo = {
            .size = sizeof(QuadVertex) * quadVertices.size(), .usage = BufferUsage::Vertex | BufferUsage::TransferDst, .memoryUsage = MemoryUsage::GPUOnly};
        spriteVertexBuffer = rhi.createBuffer(spriteVertexBufferCreateInfo);

        Buffer::CreateInfo spriteVBStagingCreateInfo = {
            .size = spriteVertexBufferCreateInfo.size, .usage = BufferUsage::TransferSrc, .memoryUsage = MemoryUsage::GPUandCPU};
        auto spriteVBStaging = rhi.createBuffer(spriteVBStagingCreateInfo);
        spriteVBStaging->setData<QuadVertex>(quadVertices);

        Buffer::CreateInfo spriteIndexBufferCreateInfo = {
            .size = sizeof(uint32_t) * quadIndices.size(), .usage = BufferUsage::Index | BufferUsage::TransferDst, .memoryUsage = MemoryUsage::GPUOnly};
        spriteIndexBuffer = rhi.createBuffer(spriteIndexBufferCreateInfo);

        Buffer::CreateInfo spriteIBStagingCreateInfo = {
            .size = spriteIndexBufferCreateInfo.size, .usage = BufferUsage::TransferSrc, .memoryUsage = MemoryUsage::GPUandCPU};
        auto spriteIBStaging = rhi.createBuffer(spriteIBStagingCreateInfo);
        spriteIBStaging->setData<uint32_t>(quadIndices);

        auto cmd = rhi.beginSingleTimeCommands();
        cmd->copyBuffer(spriteVBStaging.get(), spriteVertexBuffer.get(), spriteVertexBufferCreateInfo.size);
        cmd->copyBuffer(spriteIBStaging.get(), spriteIndexBuffer.get(), spriteIndexBufferCreateInfo.size);
        rhi.endSingleTimeCommands(cmd.get());

        std::vector<ResourceLayout::BindingCreateInfo> bindingsCreateInfo(1);
        bindingsCreateInfo[0].binding = 0;
        bindingsCreateInfo[0].type = ResourceType::StorageBuffer;
        bindingsCreateInfo[0].stages = ShaderStage::Vertex | ShaderStage::Fragment;
        bindingsCreateInfo[0].count = 1;
        spriteInstanceResourceLayout = rhi.createResourceLayout(bindingsCreateInfo);

        struct GPUInstanceData {
            Math::Mat4 modelMatrix;
            Math::Vec4 color;
            uint32_t textureIndex;
            float padding[3];
        };

        spriteInstanceResourceSets.resize(rhi.MAX_FRAMES_IN_FLIGHT);
        spriteInstanceSSBOs.resize(rhi.MAX_FRAMES_IN_FLIGHT);
        for (uint32_t i = 0; i < rhi.MAX_FRAMES_IN_FLIGHT; i++) {
            spriteInstanceResourceSets[i] = rhi.createResourceSet(spriteInstanceResourceLayout.get());

            Buffer::CreateInfo spriteInstanceSSBOCreateInfo = {
                .size = sizeof(GPUInstanceData) * MAX_SPRITE_INSTANCES_PER_FRAME, .usage = BufferUsage::Storage, .memoryUsage = MemoryUsage::GPUandCPU};
            spriteInstanceSSBOs[i] = rhi.createBuffer(spriteInstanceSSBOCreateInfo);

            Buffer* bufferData[] = {spriteInstanceSSBOs[i].get()};
            std::vector<ResourceSet::Binding> bindingsUpdateInfo(1);
            bindingsUpdateInfo[0].binding = 0;
            bindingsUpdateInfo[0].type = ResourceType::StorageBuffer;
            bindingsUpdateInfo[0].stages = ShaderStage::Vertex | ShaderStage::Fragment;
            bindingsUpdateInfo[0].data = bufferData;
            spriteInstanceResourceSets[i]->update(bindingsUpdateInfo);
        }

        std::shared_ptr<ShaderAsset> spriteShader = assetManager.getBuiltInShader("Sprite2D");

        std::vector<VertexBindingDescription> vertexBindings = {{.binding = 0, .stride = sizeof(QuadVertex), .inputRate = VertexInputRate::Vertex}};
        std::vector<VertexAttributeDescription> vertexAttributes = {
            {.location = 0, .binding = 0, .format = Format::R32G32B32Sfloat, .offset = 0},
            {.location = 1, .binding = 0, .format = Format::R32G32Sfloat, .offset = offsetof(QuadVertex, uv)}};

        Pipeline::CreateInfo pipelineCreateInfo = {
            .shader = spriteShader->getShader(),
            .vertexBindings = vertexBindings,
            .vertexAttributes = vertexAttributes,
            .topology = PrimitiveTopology::TriangleList,
            .polygonMode = PolygonMode::Fill,
            .cullMode = CullMode::None,
            .frontFaceClockwise = true,
            .enableBlending = true,
            .enableDepthTest = true,
            .enableDepthWrite = false,
            .colorAttachmentFormats = {rhi.getRenderTargetFormat()},
            .depthAttachmentFormat = rhi.getDepthTextureFormat(),
            .resourceLayouts = {globalDataResourceLayout.get(), spriteInstanceResourceLayout.get(), textureRegistry.getResourceLayout()}};
        spritePipeline = rhi.createPipeline(pipelineCreateInfo);
    }

    void ForwardRenderPipeline::createSkyboxPassData() {
        RenderAttachment colorAttachment = {.loadOp = LoadOp::Load, .storeOp = StoreOp::Store};
        RenderAttachment depthAttachment = {.loadOp = LoadOp::Load, .storeOp = StoreOp::Store};
        skyboxRenderPass = {};
        skyboxRenderPass.colorAttachments[0] = colorAttachment;
        skyboxRenderPass.colorAttachmentCount = 1;
        skyboxRenderPass.depthAttachment = depthAttachment;
        skyboxRenderPass.hasDepthAttachment = true;

        std::vector<Math::Vec3> skyboxVertices = {{-0.5f, -0.5f, -0.5f}, {0.5f, -0.5f, -0.5f}, {0.5f, 0.5f, -0.5f}, {-0.5f, 0.5f, -0.5f},
                                                  {-0.5f, -0.5f, 0.5f},  {0.5f, -0.5f, 0.5f},  {0.5f, 0.5f, 0.5f},  {-0.5f, 0.5f, 0.5f}};
        std::vector<uint32_t> skyboxIndices = {0, 1, 2, 2, 3, 0, 4, 5, 6, 6, 7, 4, 0, 4, 7, 7, 3, 0, 1, 5, 6, 6, 2, 1, 3, 2, 6, 6, 7, 3, 0, 1, 5, 5, 4, 0};

        Buffer::CreateInfo skyboxVertexBufferCreateInfo = {
            .size = sizeof(Math::Vec3) * skyboxVertices.size(), .usage = BufferUsage::Vertex | BufferUsage::TransferDst, .memoryUsage = MemoryUsage::GPUOnly};
        skyboxVertexBuffer = rhi.createBuffer(skyboxVertexBufferCreateInfo);
        Buffer::CreateInfo skyboxVertexStagingBufferCreateInfo = {
            .size = sizeof(Math::Vec3) * skyboxVertices.size(), .usage = BufferUsage::TransferSrc, .memoryUsage = MemoryUsage::GPUandCPU};
        std::unique_ptr<Buffer> skyboxVertexStagingBuffer = rhi.createBuffer(skyboxVertexStagingBufferCreateInfo);
        skyboxVertexStagingBuffer->setData<Math::Vec3>(skyboxVertices);

        Buffer::CreateInfo skyboxIndexBufferCreateInfo = {
            .size = sizeof(uint32_t) * skyboxIndices.size(), .usage = BufferUsage::Index | BufferUsage::TransferDst, .memoryUsage = MemoryUsage::GPUOnly};
        skyboxIndexBuffer = rhi.createBuffer(skyboxIndexBufferCreateInfo);
        Buffer::CreateInfo skyboxIndexStagingBufferCreateInfo = {
            .size = sizeof(uint32_t) * skyboxIndices.size(), .usage = BufferUsage::TransferSrc, .memoryUsage = MemoryUsage::GPUandCPU};
        std::unique_ptr<Buffer> skyboxIndexStagingBuffer = rhi.createBuffer(skyboxIndexStagingBufferCreateInfo);
        skyboxIndexStagingBuffer->setData<uint32_t>(skyboxIndices);

        auto commandBuffer = rhi.beginSingleTimeCommands();
        commandBuffer->copyBuffer(skyboxVertexStagingBuffer.get(), skyboxVertexBuffer.get(), skyboxVertexBufferCreateInfo.size);
        commandBuffer->copyBuffer(skyboxIndexStagingBuffer.get(), skyboxIndexBuffer.get(), skyboxIndexBufferCreateInfo.size);
        rhi.endSingleTimeCommands(commandBuffer.get());

        std::shared_ptr<ShaderAsset> skyboxShader = assetManager.getBuiltInShader("Skybox");

        std::vector<VertexBindingDescription> skyboxVertexBindings = {{.binding = 0, .stride = sizeof(Math::Vec3), .inputRate = VertexInputRate::Vertex}};
        std::vector<VertexAttributeDescription> skyboxVertexAttributes = {
            {.location = 0, .binding = 0, .format = Format::R32G32B32Sfloat, .offset = 0},
        };

        Pipeline::CreateInfo pipelineCreateInfo = {.shader = skyboxShader->getShader(),
                                                   .vertexBindings = skyboxVertexBindings,
                                                   .vertexAttributes = skyboxVertexAttributes,
                                                   .topology = PrimitiveTopology::TriangleList,
                                                   .polygonMode = PolygonMode::Fill,
                                                   .cullMode = CullMode::Front,
                                                   .frontFaceClockwise = true,
                                                   .enableBlending = false,
                                                   .enableDepthTest = true,
                                                   .enableDepthWrite = false,
                                                   .colorAttachmentFormats = {rhi.getRenderTargetFormat()},
                                                   .depthAttachmentFormat = rhi.getDepthTextureFormat(),
                                                   .resourceLayouts = {globalDataResourceLayout.get()}};
        skyboxPipeline = rhi.createPipeline(pipelineCreateInfo);
    }

    void ForwardRenderPipeline::createWorldGridPassData() {
        RenderAttachment colorAttachment = {.loadOp = LoadOp::Load, .storeOp = StoreOp::Store};
        RenderAttachment depthAttachment = {.loadOp = LoadOp::Load, .storeOp = StoreOp::Store};
        worldGridRenderPass = {};
        worldGridRenderPass.colorAttachments[0] = colorAttachment;
        worldGridRenderPass.colorAttachmentCount = 1;
        worldGridRenderPass.depthAttachment = depthAttachment;
        worldGridRenderPass.hasDepthAttachment = true;

        std::shared_ptr<ShaderAsset> worldGridShader = assetManager.getBuiltInShader("WorldGrid");

        Pipeline::CreateInfo pipelineCreateInfo = {.shader = worldGridShader->getShader(),
                                                   .vertexBindings = {},
                                                   .vertexAttributes = {},
                                                   .topology = PrimitiveTopology::TriangleList,
                                                   .polygonMode = PolygonMode::Fill,
                                                   .cullMode = CullMode::None,
                                                   .frontFaceClockwise = true,
                                                   .enableBlending = true,
                                                   .enableDepthTest = true,
                                                   .enableDepthWrite = false,
                                                   .colorAttachmentFormats = {rhi.getRenderTargetFormat()},
                                                   .depthAttachmentFormat = rhi.getDepthTextureFormat(),
                                                   .resourceLayouts = {globalDataResourceLayout.get()}};
        worldGridPipeline = rhi.createPipeline(pipelineCreateInfo);
    }

    void ForwardRenderPipeline::createGizmosPassData() {
        RenderAttachment colorAttachment = {.loadOp = LoadOp::Load, .storeOp = StoreOp::Store};
        gizmosRenderPass = {};
        gizmosRenderPass.colorAttachments[0] = colorAttachment;
        gizmosRenderPass.colorAttachmentCount = 1;

        gizmosMeshHandle = assetManager.importAsset("BuiltIn.Gizmos.Arrow", "axiom://Packages/Models/Arrow.obj", AssetType::Mesh);
        std::shared_ptr<ShaderAsset> gizmosShader = assetManager.getBuiltInShader("Gizmos");

        std::vector<VertexBindingDescription> vertexBindings = {{.binding = 0, .stride = sizeof(MeshVertex), .inputRate = VertexInputRate::Vertex}};
        std::vector<VertexAttributeDescription> vertexAttributes = {
            {.location = 0, .binding = 0, .format = Format::R32G32B32Sfloat, .offset = offsetof(MeshVertex, position)},
            {.location = 1, .binding = 0, .format = Format::R32G32B32Sfloat, .offset = offsetof(MeshVertex, normal)},
            {.location = 2, .binding = 0, .format = Format::R32G32Sfloat, .offset = offsetof(MeshVertex, uv)},
        };

        Pipeline::CreateInfo pipelineCreateInfo = {.shader = gizmosShader->getShader(),
                                                   .vertexBindings = vertexBindings,
                                                   .vertexAttributes = vertexAttributes,
                                                   .topology = PrimitiveTopology::TriangleList,
                                                   .polygonMode = PolygonMode::Fill,
                                                   .cullMode = CullMode::Back,
                                                   .frontFaceClockwise = true,
                                                   .enableBlending = true,
                                                   .enableDepthTest = false,
                                                   .enableDepthWrite = false,
                                                   .colorAttachmentFormats = {rhi.getRenderTargetFormat()},
                                                   .depthAttachmentFormat = Format::Undefined,
                                                   .resourceLayouts = {globalDataResourceLayout.get()}};
        gizmosPipeline = rhi.createPipeline(pipelineCreateInfo);
    }

    void ForwardRenderPipeline::opaquePass(Scene* scene, const Texture* renderTarget, const Texture* depthTarget, CommandBuffer* commandBuffer,
                                           uint32_t viewportIndex) {
        std::vector<DrawCommand> drawList;
        drawList.reserve(1000);

        auto view = scene->view<TransformComponent, MeshComponent>();
        for (const uint32_t entityId : view) {
            auto& transform = scene->getEntity(entityId).getComponent<TransformComponent>();
            auto& meshRenderer = scene->getEntity(entityId).getComponent<MeshComponent>();

            std::shared_ptr<MaterialAsset> material = assetManager.getAsset<MaterialAsset>(meshRenderer.meterialId);
            std::shared_ptr<MeshAsset> mesh = assetManager.getAsset<MeshAsset>(meshRenderer.meshId);

            if (material && mesh) {
                drawList.push_back({.model = transform.modelMatrix(), .meshAsset = mesh, .materialAsset = material});
            }
        }

        std::sort(drawList.begin(), drawList.end());

        std::vector<Math::Mat4> allTransforms;
        allTransforms.reserve(drawList.size());
        for (const auto& cmd : drawList) {
            allTransforms.push_back(cmd.model);
        }

        uint32_t passInstanceOffset = totalInstanceCount;
        totalInstanceCount += allTransforms.size();
        AX_CORE_ASSERT(totalInstanceCount < MAX_INSTANCES_PER_FRAME, "Instance buffer overflow!");

        instanceSSBOs[currentFrameIndex]->setData(allTransforms.data(), allTransforms.size() * sizeof(Math::Mat4), passInstanceOffset * sizeof(Math::Mat4));

        Math::iVec2 renderTargetSize = renderTarget->getSize();
        opaqueRenderPass.colorAttachments[0].texture = renderTarget;
        opaqueRenderPass.width = renderTargetSize.x();
        opaqueRenderPass.height = renderTargetSize.y();
        opaqueRenderPass.depthAttachment.texture = depthTarget;

        commandBuffer->beginRendering(opaqueRenderPass);
        commandBuffer->setViewport(0.0f, 0.0f, renderTargetSize.x(), renderTargetSize.y());
        commandBuffer->setScissor(0, 0, renderTargetSize.x(), renderTargetSize.y());
        if (drawList.empty()) {
            commandBuffer->endRendering();
            return;
        }
        commandBuffer->bindVertexBuffers({assetManager.getGlobalVertexBuffer()});
        commandBuffer->bindIndexBuffer(assetManager.getGlobalIndexBuffer());

        UUID currentShader = UUID();
        UUID currentMaterial = UUID();
        UUID currentMesh = UUID();

        uint32_t batchStartOffset = passInstanceOffset;
        uint32_t batchInstanceCount = 0;

        struct PushConstants {
            uint32_t viewportIndex;
            uint32_t instanceIndex;
        };

        auto flushBatch = [&]() {
            if (batchInstanceCount == 0) {
                return;
            }

            std::shared_ptr<MeshAsset> activeMesh = assetManager.getAsset<MeshAsset>(currentMesh);

            PushConstants pc = {.viewportIndex = viewportIndex, .instanceIndex = batchStartOffset};
            commandBuffer->bindPushConstants(&pc, sizeof(PushConstants));
            commandBuffer->drawIndexed(activeMesh->getIndexCount(), batchInstanceCount, activeMesh->getIndexOffset(), activeMesh->getVertexOffset());

            batchStartOffset += batchInstanceCount;
            batchInstanceCount = 0;
        };

        for (const auto& cmd : drawList) {
            bool stateChanged = false;
            if (cmd.meshAsset->getHandle() != currentMesh || cmd.materialAsset->getHandle() != currentMaterial) {
                stateChanged = true;
            }

            if (stateChanged) {
                flushBatch();
            }

            if (cmd.materialAsset->getShader() != currentShader) {
                currentShader = cmd.materialAsset->getShader();
                std::shared_ptr<ShaderAsset> currentShaderAsset = assetManager.getAsset<ShaderAsset>(currentShader);

                std::vector<VertexBindingDescription> meshVertexBindings = {{.binding = 0, .stride = sizeof(MeshVertex), .inputRate = VertexInputRate::Vertex}};
                std::vector<VertexAttributeDescription> meshVertexAttributes = {
                    {.location = 0, .binding = 0, .format = Format::R32G32B32Sfloat, .offset = offsetof(MeshVertex, position)},
                    {.location = 1, .binding = 0, .format = Format::R32G32B32Sfloat, .offset = offsetof(MeshVertex, normal)},
                    {.location = 2, .binding = 0, .format = Format::R32G32Sfloat, .offset = offsetof(MeshVertex, uv)},
                };

                Pipeline::CreateInfo pipelineCreateInfo = {
                    .shader = currentShaderAsset->getShader(), .vertexBindings = meshVertexBindings, .vertexAttributes = meshVertexAttributes};
                pipelineCreateInfo.colorAttachmentFormats = {renderTarget->getFormat()};
                if (depthTarget) {
                    pipelineCreateInfo.depthAttachmentFormat = depthTarget->getFormat();
                }
                pipelineCreateInfo.resourceLayouts = {globalDataResourceLayout.get(), instanceResourceLayout.get()};

                commandBuffer->bindPipeline(psoManager.getOrCreatePipeline(pipelineCreateInfo));
                commandBuffer->bindResources({globalDataResourceSets[currentFrameIndex].get(), instanceResourceSets[currentFrameIndex].get()});
            }

            if (cmd.materialAsset->getHandle() != currentMaterial) {
                // Eventually bind material-specific textures here
                currentMaterial = cmd.materialAsset->getHandle();
            }
            if (cmd.meshAsset->getHandle() != currentMesh) {
                currentMesh = cmd.meshAsset->getHandle();
            }
            batchInstanceCount++;
        }

        flushBatch();
        commandBuffer->endRendering();
    }

    void ForwardRenderPipeline::spritePass(Scene* scene, const Texture* renderTarget, const Texture* depthTarget, CommandBuffer* commandBuffer,
                                           uint32_t viewportIndex) {
        struct alignas(16) SpriteInstance {
            Math::Mat4 modelMatrix;
            Math::Vec4 color;
            uint32_t textureIndex;
            float padding[3];
        };

        std::vector<SpriteInstance> spriteInstances;
        spriteInstances.reserve(1000);

        auto view = scene->view<Sprite2DComponent, TransformComponent>();
        for (const uint32_t id : view) {
            const auto& [sprite, transform] = scene->getEntity(id).getComponents<Sprite2DComponent, TransformComponent>();

            uint32_t texIdx = textureRegistry.getOrRegister(sprite.textureId);

            spriteInstances.push_back({.modelMatrix = transform.modelMatrix(), .color = sprite.color, .textureIndex = texIdx, .padding = {0.0f, 0.0f, 0.0f}});
        }

        if (spriteInstances.empty()) {
            return;
        }

        AX_CORE_ASSERT(spriteInstances.size() < MAX_SPRITE_INSTANCES_PER_FRAME, "Sprite instance buffer overflow!");

        spriteInstanceSSBOs[currentFrameIndex]->setData(spriteInstances.data(), spriteInstances.size() * sizeof(SpriteInstance));

        Math::iVec2 renderTargetSize = renderTarget->getSize();
        spriteRenderPass.width = renderTargetSize.x();
        spriteRenderPass.height = renderTargetSize.y();
        spriteRenderPass.colorAttachments[0].texture = renderTarget;
        spriteRenderPass.depthAttachment.texture = depthTarget;

        commandBuffer->beginRendering(spriteRenderPass);
        commandBuffer->setViewport(0.0f, 0.0f, renderTargetSize.x(), renderTargetSize.y());
        commandBuffer->setScissor(0, 0, renderTargetSize.x(), renderTargetSize.y());

        commandBuffer->bindPipeline(spritePipeline.get());
        commandBuffer->bindVertexBuffers({spriteVertexBuffer.get()});
        commandBuffer->bindIndexBuffer(spriteIndexBuffer.get());

        commandBuffer->bindResources(
            {globalDataResourceSets[currentFrameIndex].get(), spriteInstanceResourceSets[currentFrameIndex].get(), textureRegistry.getResourceSet()});

        struct PushConstants {
            uint32_t viewportIndex;
        } pc{.viewportIndex = viewportIndex};
        commandBuffer->bindPushConstants(&pc, sizeof(PushConstants));

        commandBuffer->drawIndexed(6, static_cast<uint32_t>(spriteInstances.size()), 0, 0, 0);

        commandBuffer->endRendering();
    }

    void ForwardRenderPipeline::skyboxPass(Scene* scene, const Texture* renderTarget, const Texture* depthTarget, CommandBuffer* commandBuffer) {
        Math::iVec2 renderTargetSize = renderTarget->getSize();
        skyboxRenderPass.width = renderTargetSize.x();
        skyboxRenderPass.height = renderTargetSize.y();
        skyboxRenderPass.colorAttachments[0].texture = renderTarget;
        skyboxRenderPass.depthAttachment.texture = depthTarget;

        commandBuffer->beginRendering(skyboxRenderPass);
        commandBuffer->setViewport(0.0f, 0.0f, renderTargetSize.x(), renderTargetSize.y());
        commandBuffer->setScissor(0, 0, renderTargetSize.x(), renderTargetSize.y());
        commandBuffer->bindPipeline(skyboxPipeline.get());
        commandBuffer->bindVertexBuffers({skyboxVertexBuffer.get()});
        commandBuffer->bindIndexBuffer(skyboxIndexBuffer.get());
        commandBuffer->bindResources({globalDataResourceSets[currentFrameIndex].get()});
        commandBuffer->drawIndexed(36, 1);
        commandBuffer->endRendering();
    }

    void ForwardRenderPipeline::worldGridPass(Scene* scene, const Texture* renderTarget, const Texture* depthTarget, CommandBuffer* commandBuffer) {
        Math::iVec2 renderTargetSize = renderTarget->getSize();
        worldGridRenderPass.width = renderTargetSize.x();
        worldGridRenderPass.height = renderTargetSize.y();
        worldGridRenderPass.colorAttachments[0].texture = renderTarget;
        worldGridRenderPass.depthAttachment.texture = depthTarget;

        commandBuffer->beginRendering(worldGridRenderPass);
        commandBuffer->setViewport(0.0f, 0.0f, renderTargetSize.x(), renderTargetSize.y());
        commandBuffer->setScissor(0, 0, renderTargetSize.x(), renderTargetSize.y());
        commandBuffer->bindPipeline(worldGridPipeline.get());
        commandBuffer->bindResources({globalDataResourceSets[currentFrameIndex].get()});
        commandBuffer->draw(6, 1);
        commandBuffer->endRendering();
    }

    void ForwardRenderPipeline::gizmosPass(Scene* scene, const Texture* renderTarget, const Texture* depthTarget, CommandBuffer* commandBuffer,
                                           const Math::Vec3& gizmoPosition) {
        struct PushConstants {
            Math::Mat4 model;
            Math::Vec4 color;
        };

        PushConstants pushConstants = {.model = Math::Mat4::identity(), .color = Color::white()};
        float distanceToTheCamera = Math::length(globalData.cameraPosition - gizmoPosition);
        float scale = distanceToTheCamera * 0.05f;

        Math::iVec2 renderTargetSize = renderTarget->getSize();
        gizmosRenderPass.colorAttachments[0].texture = renderTarget;
        gizmosRenderPass.width = renderTargetSize.x();
        gizmosRenderPass.height = renderTargetSize.y();

        std::shared_ptr<MeshAsset> gizmosMesh = assetManager.getAsset<MeshAsset>(gizmosMeshHandle);

        commandBuffer->beginRendering(gizmosRenderPass);
        commandBuffer->bindPipeline(gizmosPipeline.get());
        commandBuffer->bindVertexBuffers({assetManager.getGlobalVertexBuffer()});
        commandBuffer->bindIndexBuffer(assetManager.getGlobalIndexBuffer());
        commandBuffer->setViewport(0.0f, 0.0f, renderTargetSize.x(), renderTargetSize.y());
        commandBuffer->setScissor(0, 0, renderTargetSize.x(), renderTargetSize.y());
        commandBuffer->bindResources({globalDataResourceSets[currentFrameIndex].get()});

        // Y axis
        pushConstants.model = Math::Mat4::model(gizmoPosition, Math::Vec3::zero(), Math::Vec3(scale));
        pushConstants.color = Color::green();
        commandBuffer->bindPushConstants(&pushConstants, sizeof(PushConstants));
        commandBuffer->drawIndexed(gizmosMesh->getIndexCount(), 1, gizmosMesh->getIndexOffset(), gizmosMesh->getVertexOffset(), 0);
        // X axis
        pushConstants.model = Math::Mat4::model(gizmoPosition, Math::Vec3(0.0f, 0.0f, Math::PI * -0.5f), Math::Vec3(scale));
        pushConstants.color = Color::red();
        commandBuffer->bindPushConstants(&pushConstants, sizeof(PushConstants));
        commandBuffer->drawIndexed(gizmosMesh->getIndexCount(), 1, gizmosMesh->getIndexOffset(), gizmosMesh->getVertexOffset(), 0);
        // Z axis
        pushConstants.model = Math::Mat4::model(gizmoPosition, Math::Vec3(Math::PI * 0.5f, 0.0f, 0.0f), Math::Vec3(scale));
        pushConstants.color = Color::blue();
        commandBuffer->bindPushConstants(&pushConstants, sizeof(PushConstants));
        commandBuffer->drawIndexed(gizmosMesh->getIndexCount(), 1, gizmosMesh->getIndexOffset(), gizmosMesh->getVertexOffset(), 0);

        commandBuffer->endRendering();
    }

    void ForwardRenderPipeline::injectRenderFeatures(RenderInjectionPoint injectionPoint, RenderGraph& renderGraph, const RenderContext& context) {
        for (const auto& feature : renderFeatures) {
            if (feature->getInjectionPoint() == injectionPoint) {
                feature->injectPasses(renderGraph, context);
            }
        }
    }
} // namespace Axiom
