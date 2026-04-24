#include "AssetManager.h"
#include "AxImageLoader.h"
#include "AxModelLoader.h"
#include "Core/Application.h"

namespace Axiom {
    std::unordered_map<UUID, std::shared_ptr<Asset>> AssetManager::assets;
    std::unordered_map<std::string, UUID> AssetManager::assetHandles;

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

            Buffer::CreateInfo vertexBufferCreateInfo = {
                .size = vertices.size() * sizeof(MeshVertex), .usage = BufferUsage::Vertex, .memoryUsage = MemoryUsage::GPUOnly};
            std::unique_ptr<Buffer> vertexBuffer = Application::getRenderer()->createBuffer(vertexBufferCreateInfo);

            Buffer::CreateInfo indexBufferCreateInfo = {
                .size = modelResult->indices.size() * sizeof(uint32_t), .usage = BufferUsage::Index, .memoryUsage = MemoryUsage::GPUOnly};
            std::unique_ptr<Buffer> indexBuffer = Application::getRenderer()->createBuffer(indexBufferCreateInfo);

            Buffer::CreateInfo stagingVertexBufferCreateInfo = {
                .size = vertices.size() * sizeof(MeshVertex), .usage = BufferUsage::TransferSrc, .memoryUsage = MemoryUsage::GPUandCPU};
            std::unique_ptr<Buffer> stagingVertexBuffer = Application::getRenderer()->createBuffer(stagingVertexBufferCreateInfo);
            stagingVertexBuffer->setData(vertices.data(), vertices.size() * sizeof(MeshVertex));

            Buffer::CreateInfo stagingIndexBufferCreateInfo = {
                .size = modelResult->indices.size() * sizeof(uint32_t), .usage = BufferUsage::TransferSrc, .memoryUsage = MemoryUsage::GPUandCPU};
            std::unique_ptr<Buffer> stagingIndexBuffer = Application::getRenderer()->createBuffer(stagingIndexBufferCreateInfo);
            stagingIndexBuffer->setData(modelResult->indices.data(), modelResult->indices.size() * sizeof(uint32_t));

            std::unique_ptr<CommandBuffer> commandBuffer = Application::getRenderer()->beginSingleTimeCommands();
            commandBuffer->copyBuffer(stagingVertexBuffer.get(), vertexBuffer.get(), vertices.size() * sizeof(MeshVertex));
            commandBuffer->copyBuffer(stagingIndexBuffer.get(), indexBuffer.get(), modelResult->indices.size() * sizeof(uint32_t));
            Application::getRenderer()->endSingleTimeCommands(commandBuffer.get());

            assets[handle] =
                std::make_shared<MeshAsset>(handle, path.filename().string(), std::move(vertexBuffer), std::move(indexBuffer), modelResult->indices.size());
            assetHandles[cacheString] = handle;
            return handle;
        }

        AX_CORE_LOG_ERROR("Failed to load mesh: {}", modelResult.error());
        return 0;
    }
} // namespace Axiom
