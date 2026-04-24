#include "AssetManager.h"
#include "AxImageLoader.h"
#include "AxModelLoader.h"
#include "Core/Application.h"

namespace Axiom {
    std::unordered_map<UUID, std::shared_ptr<Asset>> AssetManager::assets;
    std::unordered_map<std::string, UUID> AssetManager::assetHandles;
    std::unique_ptr<Buffer> AssetManager::globalVertexBuffer;
    std::unique_ptr<Buffer> AssetManager::globalIndexBuffer;
    uint32_t AssetManager::currentVertexCount = 0;
    uint32_t AssetManager::currentIndexCount = 0;

    UUID AssetManager::loadTexture(const std::filesystem::path& path) {
        std::string cacheString = path.generic_string();
        if (assetHandles.find(cacheString) != assetHandles.end()) {
            return assetHandles[cacheString];
        }

        UUID handle = UUID();

        auto imageResult = AxImageLoader::loadImage(path, 4);

        if (imageResult.has_value()) {
            Texture::CreateInfo createInfo = {.width = imageResult->width,
                                              .height = imageResult->height,
                                              .mipLevels = 1,
                                              .arrayLayers = 1,
                                              .format = Format::R8G8B8A8Unorm,
                                              .usage = TextureUsage::Sampled | TextureUsage::TransferDst,
                                              .aspect = TextureAspect::Color,
                                              .initialState = TextureState::Undefined,
                                              .memoryUsage = MemoryUsage::GPUOnly};

            std::unique_ptr<Texture> texture = Application::getRenderer()->createTexture(createInfo);

            Buffer::CreateInfo stagingBufferCreateInfo = {
                .size = imageResult->data.size(), .usage = BufferUsage::TransferSrc, .memoryUsage = MemoryUsage::GPUandCPU};
            std::unique_ptr<Buffer> stagingBuffer = Application::getRenderer()->createBuffer(stagingBufferCreateInfo);
            stagingBuffer->setData(imageResult->data.data(), imageResult->data.size());

            std::unique_ptr<CommandBuffer> commandBuffer = Application::getRenderer()->beginSingleTimeCommands();
            commandBuffer->copyBufferToTexture(stagingBuffer.get(), texture.get(), imageResult->width, imageResult->height);
            Application::getRenderer()->endSingleTimeCommands(commandBuffer.get());

            assets[handle] = std::make_shared<TextureAsset>(handle, path.filename().string(), std::move(texture));
            assetHandles[cacheString] = handle;
            return handle;
        }

        AX_CORE_LOG_ERROR("Failed to load texture: {}", imageResult.error());

        return 0;
    }

    UUID AssetManager::loadShader(const std::filesystem::path& path) {
        std::string cacheString = path.generic_string();
        if (assetHandles.find(cacheString) != assetHandles.end()) {
            return assetHandles[cacheString];
        }

        UUID handle = UUID();

        auto source = FileSystem::readFileStr(path);

        size_t vertexPos = source.find("#type vertex");
        size_t fragmentPos = source.find("#type fragment");

        std::string vertexSource = source.substr(vertexPos + 13, fragmentPos - (vertexPos + 13));
        std::string fragmentSource = source.substr(fragmentPos + 15, std::string::npos);

        std::unique_ptr<Shader> shader = Application::getRenderer()->createShader(vertexSource, fragmentSource);
        assets[handle] = std::make_shared<ShaderAsset>(handle, path.filename().string(), std::move(shader));
        assetHandles[cacheString] = handle;
        return handle;
    }

    UUID AssetManager::loadMesh(const std::filesystem::path& path) {
        std::string cacheString = path.generic_string();
        if (assetHandles.find(cacheString) != assetHandles.end()) {
            return assetHandles[cacheString];
        }

        UUID handle = UUID();

        auto modelResult = AxModelLoader::loadModel(path);

        if (modelResult.has_value()) {
            std::vector<MeshVertex> vertices;
            vertices.reserve(modelResult->vertices.size() / 3);
            for (size_t i = 0; i < modelResult->vertices.size() / 3; i++) {
                MeshVertex vertex;
                vertex.position = {modelResult->vertices[i * 3], modelResult->vertices[i * 3 + 1], modelResult->vertices[i * 3 + 2]};
                vertex.normal = {modelResult->normals[i * 3], modelResult->normals[i * 3 + 1], modelResult->normals[i * 3 + 2]};
                vertex.uv = {modelResult->texCoords[i * 2], modelResult->texCoords[i * 2 + 1]};
                vertices.push_back(vertex);
            }

            uint32_t vertexCount = vertices.size();
            uint32_t indexCount = modelResult->indices.size();
            uint32_t vertexBytes = vertexCount * sizeof(MeshVertex);
            uint32_t indexBytes = indexCount * sizeof(uint32_t);

            Buffer::CreateInfo vertexStagingInfo = {.size = vertexBytes, .usage = BufferUsage::TransferSrc, .memoryUsage = MemoryUsage::GPUandCPU};
            std::unique_ptr<Buffer> vertexStaging = Application::getRenderer()->createBuffer(vertexStagingInfo);
            vertexStaging->setData(vertices.data(), vertexBytes);

            Buffer::CreateInfo indexStagingInfo = {.size = indexBytes, .usage = BufferUsage::TransferSrc, .memoryUsage = MemoryUsage::GPUandCPU};
            std::unique_ptr<Buffer> indexStaging = Application::getRenderer()->createBuffer(indexStagingInfo);
            indexStaging->setData(modelResult->indices.data(), indexBytes);

            auto commandBuffer = Application::getRenderer()->beginSingleTimeCommands();
            uint32_t vertexByteDstOffset = currentVertexCount * sizeof(MeshVertex);
            uint32_t indexByteDstOffset = currentIndexCount * sizeof(uint32_t);
            commandBuffer->copyBuffer(vertexStaging.get(), globalVertexBuffer.get(), vertexBytes, vertexByteDstOffset);
            commandBuffer->copyBuffer(indexStaging.get(), globalIndexBuffer.get(), indexBytes, indexByteDstOffset);
            Application::getRenderer()->endSingleTimeCommands(commandBuffer.get());

            assets[handle] = std::make_shared<MeshAsset>(handle, path.filename().string(), currentVertexCount, currentIndexCount, modelResult->indices.size());
            assetHandles[cacheString] = handle;

            currentVertexCount += vertexCount;
            currentIndexCount += indexCount;
            return handle;
        }

        AX_CORE_LOG_ERROR("Failed to load mesh: {}", modelResult.error());
        return 0;
    }

    void AssetManager::init() {
        uint64_t globalBufferSize = Math::megabytes(512);

        Buffer::CreateInfo vertexBufferCreateInfo = {
            .size = globalBufferSize, .usage = BufferUsage::Vertex | BufferUsage::TransferDst, .memoryUsage = MemoryUsage::GPUOnly};
        globalVertexBuffer = Application::getRenderer()->createBuffer(vertexBufferCreateInfo);
        Buffer::CreateInfo indexBufferCreateInfo = {
            .size = globalBufferSize, .usage = BufferUsage::Index | BufferUsage::TransferDst, .memoryUsage = MemoryUsage::GPUOnly};
        globalIndexBuffer = Application::getRenderer()->createBuffer(indexBufferCreateInfo);
    }

    void AssetManager::shutdown() {
        assets.clear();
        assetHandles.clear();

        globalVertexBuffer.reset();
        globalIndexBuffer.reset();
    }
} // namespace Axiom
