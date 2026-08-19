#include "axpch.h"

#include "AssetManager.h"

#include "Asset/Asset.h"
#include "Asset/MaterialAsset.h"
#include "Asset/UUID.h"
#include "AxImageLoader.h"
#include "AxModelLoader.h"
#include "Core/Locator.h"
#include "Core/Log.h"
#include "Core/TaskManager.h"
#include "Math/Color.h"
#include "MeshAsset.h"
#include "Renderer/Renderer.h"
#include "Renderer/Shader.h"
#include "ShaderAsset.h"
#include "TextureAsset.h"
#include "Utils/FileSystem.h"
#include "Utils/JSONSerializer.h"

#include <string>

namespace Axiom {
    std::unordered_map<UUID, AssetMetadata> AssetManager::registry;
    std::unordered_map<UUID, std::shared_ptr<Asset>> AssetManager::loadedAssets;
    std::unordered_map<std::string, UUID> AssetManager::assetHandles;

    std::unique_ptr<Buffer> AssetManager::globalVertexBuffer;
    std::unique_ptr<Buffer> AssetManager::globalIndexBuffer;
    uint32_t AssetManager::currentVertexCount = 0;
    uint32_t AssetManager::currentIndexCount = 0;

    UUID AssetManager::importAsset(const std::string& name, const std::filesystem::path& path, AssetType type) {
        std::string cacheString = path.generic_string();
        if (assetHandles.find(cacheString) != assetHandles.end()) {
            return assetHandles[cacheString];
        }

        if (!FileSystem::exists(path)) {
            AX_CORE_LOG_ERROR("Tried to import an asset that does not exist: {}", path.generic_string());
            return UUID();
        }

        UUID newID = UUID::generate();
        AssetMetadata meta = {.name = name, .type = type, .filePath = path};

        registry[newID] = meta;
        assetHandles[cacheString] = newID;

        return newID;
    }

    void AssetManager::init() {
        uint64_t globalBufferSize = Math::megabytes(512);

        Buffer::CreateInfo vertexBufferCreateInfo = {
            .size = globalBufferSize, .usage = BufferUsage::Vertex | BufferUsage::TransferDst, .memoryUsage = MemoryUsage::GPUOnly};
        globalVertexBuffer = Locator::getRenderer()->createBuffer(vertexBufferCreateInfo);
        Buffer::CreateInfo indexBufferCreateInfo = {
            .size = globalBufferSize, .usage = BufferUsage::Index | BufferUsage::TransferDst, .memoryUsage = MemoryUsage::GPUOnly};
        globalIndexBuffer = Locator::getRenderer()->createBuffer(indexBufferCreateInfo);

        deserializeManifest("axiom://Packages/Packages.json");

        initDefaultAssets();
    }

    void AssetManager::shutdown() {
        loadedAssets.clear();
        assetHandles.clear();

        globalVertexBuffer.reset();
        globalIndexBuffer.reset();

        serializeManifest("axiom://Packages/Packages.json", "axiom://");

        registry.clear();
    }

    void AssetManager::serializeManifest(const std::filesystem::path& manifestPath, const std::string& vfsFilter) {
        JSONValue manifestRoot;
        JSONValue manifestAssets;

        for (const auto& [uuid, meta] : registry) {
            if (!meta.filePath.generic_string().starts_with(vfsFilter)) {
                continue;
            }

            JSONValue asset;

            JSONValue nameNode;
            nameNode.setString(meta.name);
            JSONValue filePathNode;
            filePathNode.setString(meta.filePath.generic_string());
            JSONValue typeNode;
            typeNode.setInt(static_cast<int>(meta.type));

            asset.setChild("Name", nameNode);
            asset.setChild("FilePath", filePathNode);
            asset.setChild("Type", typeNode);

            manifestAssets.setChild(std::to_string(uuid), asset);
        }

        manifestRoot.setChild("Assets", manifestAssets);
        FileSystem::writeFile(manifestPath, JSONSerializer::serialize(manifestRoot));
    }

    void AssetManager::deserializeManifest(const std::filesystem::path& manifestPath) {
        std::string manifestJsonString = FileSystem::readFileStr(manifestPath);
        if (!manifestJsonString.empty()) {
            JSONValue root = JSONSerializer::deserialize(manifestJsonString);
            if (root.getType() == JSONValueType::Object && root.hasChild("Assets")) {
                const JSONValue& assetsNode = root.getChild("Assets");
                const auto& children = assetsNode.getChildren();

                for (const auto& [uuidStr, node] : children) {
                    uint64_t uuidValue = std::stoull(uuidStr);
                    const auto& assetData = node.getChildren();
                    std::string name = assetData.at("Name").getString();
                    std::string rawPath = assetData.at("FilePath").getString();
                    AssetType type = static_cast<AssetType>(assetData.at("Type").getInt());
                    std::string cacheString = std::filesystem::path(rawPath).generic_string();

                    AssetMetadata metaData = {.name = name, .type = type, .filePath = std::filesystem::path(cacheString)};
                    registry[UUID(uuidValue)] = metaData;
                    assetHandles[cacheString] = UUID(uuidValue);
                }
            }
        }
    }

    void AssetManager::initDefaultAssets() {
        // Default texture
        const uint32_t defaultTextureSize = 4;

        Texture::CreateInfo textureCreateInfo = {.width = defaultTextureSize,
                                                 .height = defaultTextureSize,
                                                 .mipLevels = 1,
                                                 .arrayLayers = 1,
                                                 .format = Format::R8G8B8A8Unorm,
                                                 .usage = TextureUsage::Sampled | TextureUsage::TransferDst,
                                                 .aspect = TextureAspect::Color,
                                                 .initialState = TextureState::Undefined,
                                                 .memoryUsage = MemoryUsage::GPUOnly};
        std::unique_ptr<Texture> defaultTexture = Locator::getRenderer()->createTexture(textureCreateInfo);

        Buffer::CreateInfo stagingBufferCreateInfo = {
            .size = defaultTextureSize * defaultTextureSize * sizeof(uint32_t), .usage = BufferUsage::TransferSrc, .memoryUsage = MemoryUsage::GPUandCPU};
        std::unique_ptr<Buffer> stagingBuffer = Locator::getRenderer()->createBuffer(stagingBufferCreateInfo);

        // Magenta: R=255, G=0, B=255, A=255
        // Black:   R=0,   G=0, B=0,   A=255
        const uint32_t magenta = 0xFF00FFFF; // AABBGGRR (check your API's expected byte order)
        const uint32_t black = 0xFF000000;

        std::array<uint32_t, defaultTextureSize * defaultTextureSize> defaultTextureData;
        for (uint32_t y = 0; y < defaultTextureSize; y++) {
            for (uint32_t x = 0; x < defaultTextureSize; x++) {
                if ((x + y) % 2 == 0) {
                    defaultTextureData[y * defaultTextureSize + x] = magenta;
                } else {
                    defaultTextureData[y * defaultTextureSize + x] = black;
                }
            }
        }

        std::unique_ptr<CommandBuffer> commandBuffer = Locator::getRenderer()->beginSingleTimeCommands();
        stagingBuffer->setData<uint32_t>(defaultTextureData);
        commandBuffer->copyBufferToTexture(stagingBuffer.get(), defaultTexture.get(), defaultTextureSize, defaultTextureSize);
        Locator::getRenderer()->endSingleTimeCommands(commandBuffer.get());

        AssetMetadata defaultTextureMeta = {.name = "BuiltIn.DefaultTexture", .type = AssetType::Texture, .filePath = ""};
        registry[defaultTextureHandle] = defaultTextureMeta;
        loadedAssets[defaultTextureHandle] = std::make_shared<TextureAsset>(defaultMaterialHandle, "BuiltIn.DefaultTexture", std::move(defaultTexture));

        // Default material
        std::filesystem::path defaultShaderPath = "axiom://Packages/Shaders/BuiltIn.DefaultPBR.axs";
        UUID defaultShaderHandle = importAsset("BuiltIn.DefaultPBR", defaultShaderPath, AssetType::Shader);

        auto material = std::make_shared<MaterialAsset>(defaultMaterialHandle, "BuiltIn.DefaultPBR", defaultShaderHandle, nullptr);
        material->setAlbedoColor(Color::lightGray());

        AssetMetadata defaultMaterialMeta = {.name = "BuiltIn.DefaultMaterial", .type = AssetType::Material, .filePath = ""};
        registry[defaultMaterialHandle] = defaultMaterialMeta;
        loadedAssets[defaultMaterialHandle] = material;

        // Default Mesh
        const std::vector<MeshVertex> cubeVertices = {// Front face (Z = 0.5)
                                                      {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
                                                      {{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},
                                                      {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
                                                      {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
                                                      // Back face (Z = -0.5)
                                                      {{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f}},
                                                      {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f}},
                                                      {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f}},
                                                      {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f}},
                                                      // Left face (X = -0.5)
                                                      {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
                                                      {{-0.5f, -0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
                                                      {{-0.5f, 0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
                                                      {{-0.5f, 0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
                                                      // Right face (X = 0.5)
                                                      {{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f}},
                                                      {{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
                                                      {{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f}},
                                                      {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f}},
                                                      // Top face (Y = 0.5)
                                                      {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
                                                      {{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f}},
                                                      {{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f}},
                                                      {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f}},
                                                      // Bottom face (Y = -0.5)
                                                      {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f}},
                                                      {{0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f}},
                                                      {{0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f}},
                                                      {{-0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f}}};

        const std::vector<uint32_t> cubeIndices = {
            0,  1,  2,  2,  3,  0,  // Front
            4,  5,  6,  6,  7,  4,  // Back
            8,  9,  10, 10, 11, 8,  // Left
            12, 13, 14, 14, 15, 12, // Right
            16, 17, 18, 18, 19, 16, // Top
            20, 21, 22, 22, 23, 20  // Bottom
        };

        uint32_t vertexBytes = cubeVertices.size() * sizeof(MeshVertex);
        uint32_t indexBytes = cubeIndices.size() * sizeof(uint32_t);

        Buffer::CreateInfo vertexStagingInfo = {.size = vertexBytes, .usage = BufferUsage::TransferSrc, .memoryUsage = MemoryUsage::GPUandCPU};
        std::unique_ptr<Buffer> vertexStaging = Locator::getRenderer()->createBuffer(vertexStagingInfo);
        vertexStaging->setData(cubeVertices.data(), vertexBytes);

        Buffer::CreateInfo indexStagingInfo = {.size = indexBytes, .usage = BufferUsage::TransferSrc, .memoryUsage = MemoryUsage::GPUandCPU};
        std::unique_ptr<Buffer> indexStaging = Locator::getRenderer()->createBuffer(indexStagingInfo);
        indexStaging->setData(cubeIndices.data(), indexBytes);

        auto cubeCommandBuffer = Locator::getRenderer()->beginSingleTimeCommands();
        cubeCommandBuffer->copyBuffer(vertexStaging.get(), globalVertexBuffer.get(), vertexBytes, 0);
        cubeCommandBuffer->copyBuffer(indexStaging.get(), globalIndexBuffer.get(), indexBytes, 0);
        Locator::getRenderer()->endSingleTimeCommands(cubeCommandBuffer.get());

        currentVertexCount += cubeVertices.size();
        currentIndexCount += cubeIndices.size();

        AssetMetadata defaultMeshMeta = {.name = "Default Mesh", .type = AssetType::Mesh, .filePath = ""};
        registry[defaultMeshHandle] = defaultMeshMeta;

        loadedAssets[defaultMeshHandle] = std::make_shared<MeshAsset>(defaultMeshHandle, "Default Mesh", 0, 0, cubeIndices.size());

        // Error shader
        const std::string errorShaderSource = R"glsl(
        #type vertex
        #version 460
        #pragma shader_stage(vertex)

        layout(location = 0) in vec3 aPosition;
        layout(location = 1) in vec3 aNormal;
        layout(location = 2) in vec2 aTexCoord;

        layout(push_constant) uniform PushConstants {
            uint viewportIndex;
            uint instanceOffset;
        } pushConstants;

        struct GlobalData {
            mat4 uView;
            mat4 uProjection;
            vec4 uCameraPosition;
            vec4 uAmbientColor;
            vec4 uDirectionalLightDir;
            vec4 uDirectionalLightColor;
        };

        layout(std140, set = 0, binding = 0) uniform GlobalDataBuffer {
            GlobalData viewports[10];
        } globalDataBuffer;

        struct MeshInstance {
            mat4 model;
        };

        layout(std430, set = 1, binding = 1) readonly buffer InstanceBuffer {
            MeshInstance instances[];
        };

        void main() {
            MeshInstance instance = instances[gl_InstanceIndex + pushConstants.instanceOffset];

            gl_Position = globalDataBuffer.viewports[pushConstants.viewportIndex].uProjection *
                          globalDataBuffer.viewports[pushConstants.viewportIndex].uView *
                          instance.model * vec4(aPosition, 1.0);
        }

        #type fragment
        #version 460
        #pragma shader_stage(fragment)

        layout(location = 0) out vec4 oColor;

        void main() {
            oColor = vec4(1.0, 0.0, 1.0, 1.0);
        }
        )glsl";

        size_t vertexPos = errorShaderSource.find("#type vertex");
        size_t fragmentPos = errorShaderSource.find("#type fragment");

        std::string vertexSource = errorShaderSource.substr(vertexPos + 13, fragmentPos - (vertexPos + 13));
        std::string fragmentSource = errorShaderSource.substr(fragmentPos + 15, std::string::npos);

        std::unique_ptr<Shader> errorShader = Locator::getRenderer()->createShader(vertexSource, fragmentSource);
        AssetMetadata errorShaderMeta = {.name = "Error Shader", .type = AssetType::Shader, .filePath = ""};
        registry[errorShaderHandle] = errorShaderMeta;
        loadedAssets[errorShaderHandle] = std::make_shared<ShaderAsset>(errorShaderHandle, "Error Shader", std::move(errorShader));
    }

    std::shared_ptr<Asset> AssetManager::loadTexture(const std::filesystem::path& path, UUID uuid) {
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

            std::unique_ptr<Texture> texture = Locator::getRenderer()->createTexture(createInfo);

            Buffer::CreateInfo stagingBufferCreateInfo = {
                .size = imageResult->data.size(), .usage = BufferUsage::TransferSrc, .memoryUsage = MemoryUsage::GPUandCPU};
            std::unique_ptr<Buffer> stagingBuffer = Locator::getRenderer()->createBuffer(stagingBufferCreateInfo);
            stagingBuffer->setData(imageResult->data.data(), imageResult->data.size());

            std::unique_ptr<CommandBuffer> commandBuffer = Locator::getRenderer()->beginSingleTimeCommands();
            commandBuffer->copyBufferToTexture(stagingBuffer.get(), texture.get(), imageResult->width, imageResult->height);
            Locator::getRenderer()->endSingleTimeCommands(commandBuffer.get());

            return std::make_shared<TextureAsset>(uuid, path.filename().string(), std::move(texture));
        }

        AX_CORE_LOG_ERROR("Failed to load texture({}): {}", uint64_t(uuid), imageResult.error());
        return nullptr;
    }

    std::shared_ptr<Asset> AssetManager::loadShader(const std::filesystem::path& path, UUID uuid) {
        loadedAssets[uuid] = getErrorShader();

        auto source = FileSystem::readFileStr(path);
        if (source.empty()) {
            AX_CORE_LOG_ERROR_ONCE("Failed to read shader file: {}", path.generic_string());
            return loadedAssets[uuid];
        }

        size_t vertexPos = source.find("#type vertex");
        size_t fragmentPos = source.find("#type fragment");
        if (vertexPos == std::string::npos || fragmentPos == std::string::npos) {
            AX_CORE_LOG_ERROR_ONCE("Shader {} is missing #type vertex or #type fragment tags.", path.generic_string());
            return loadedAssets[uuid];
        }

        std::string vertexSource = source.substr(vertexPos + 13, fragmentPos - (vertexPos + 13));
        std::string fragmentSource = source.substr(fragmentPos + 15, std::string::npos);

        std::unique_ptr<Shader> shader = Locator::getRenderer()->createShader(vertexSource, fragmentSource);
        if (shader->isCompiled()) {
            loadedAssets[uuid] = std::make_shared<ShaderAsset>(uuid, path.filename().string(), std::move(shader));
        }
        return loadedAssets[uuid];
    }

    std::shared_ptr<Asset> AssetManager::loadMesh(const std::filesystem::path& path, UUID uuid) {
        loadedAssets[uuid] = getDefaultMesh();

        std::thread([path, uuid]() {
            auto modelResult = AxModelLoader::loadModel(path);
            if (!modelResult.has_value()) {
                AX_CORE_LOG_ERROR_ONCE("Failed to load mesh {}", modelResult.error());
                return;
            }

            std::vector<MeshVertex> vertices;
            vertices.reserve(modelResult->vertices.size() / 3);
            for (size_t i = 0; i < modelResult->vertices.size() / 3; i++) {
                MeshVertex vertex;
                vertex.position = {modelResult->vertices[i * 3], modelResult->vertices[i * 3 + 1], modelResult->vertices[i * 3 + 2]};
                vertex.normal = {modelResult->normals[i * 3], modelResult->normals[i * 3 + 1], modelResult->normals[i * 3 + 2]};
                vertex.uv = {modelResult->texCoords[i * 2], modelResult->texCoords[i * 2 + 1]};
                vertices.push_back(vertex);
            }

            Locator::getTaskManager()->submitToMain([uuid, path, vertices = std::move(vertices), indices = std::move(modelResult->indices)]() {
                uint32_t vertexCount = vertices.size();
                uint32_t indexCount = indices.size();
                uint32_t vertexBytes = vertexCount * sizeof(MeshVertex);
                uint32_t indexBytes = indexCount * sizeof(uint32_t);

                Buffer::CreateInfo vertexStagingInfo = {.size = vertexBytes, .usage = BufferUsage::TransferSrc, .memoryUsage = MemoryUsage::GPUandCPU};
                std::unique_ptr<Buffer> vertexStaging = Locator::getRenderer()->createBuffer(vertexStagingInfo);
                vertexStaging->setData(vertices.data(), vertexBytes);

                Buffer::CreateInfo indexStagingInfo = {.size = indexBytes, .usage = BufferUsage::TransferSrc, .memoryUsage = MemoryUsage::GPUandCPU};
                std::unique_ptr<Buffer> indexStaging = Locator::getRenderer()->createBuffer(indexStagingInfo);
                indexStaging->setData(indices.data(), indexBytes);

                auto commandBuffer = Locator::getRenderer()->beginSingleTimeCommands();
                uint32_t vertexByteDstOffset = currentVertexCount * sizeof(MeshVertex);
                uint32_t indexByteDstOffset = currentIndexCount * sizeof(uint32_t);
                commandBuffer->copyBuffer(vertexStaging.get(), globalVertexBuffer.get(), vertexBytes, vertexByteDstOffset);
                commandBuffer->copyBuffer(indexStaging.get(), globalIndexBuffer.get(), indexBytes, indexByteDstOffset);
                Locator::getRenderer()->endSingleTimeCommands(commandBuffer.get());

                currentVertexCount += vertexCount;
                currentIndexCount += indexCount;

                loadedAssets[uuid] = std::make_shared<MeshAsset>(uuid, path.filename().string(), currentVertexCount - vertexCount,
                                                                 currentIndexCount - indexCount, indices.size());
            });
        }).detach();

        return loadedAssets[uuid];
    }

    std::shared_ptr<Asset> AssetManager::loadMaterial(const std::filesystem::path& path, UUID uuid) {
        std::string fileContent = FileSystem::readFileStr(path);
        if (fileContent.empty()) {
            AX_CORE_LOG_ERROR("Failed to read material file: {}", path.generic_string());
            return nullptr;
        }

        JSONValue root = JSONSerializer::deserialize(fileContent);
        if (root.getType() != JSONValueType::Object) {
            AX_CORE_LOG_ERROR("Invalid material file format: {}", path.generic_string());
            return nullptr;
        }

        std::string name = path.filename().string();
        if (root.hasChild("Name")) {
            name = root.getChild("Name").getString();
        }

        UUID shaderHandle = UUID();
        if (root.hasChild("Shader")) {
            shaderHandle = UUID(std::stoull(root.getChild("Shader").getString()));
        }

        if (!shaderHandle.isValid()) {
            AX_CORE_LOG_ERROR("Material {} is missing a valid Shader UUID!", path.generic_string());
            // return nullptr;
        }

        auto material = std::make_shared<MaterialAsset>(uuid, name, shaderHandle, nullptr);

        if (root.hasChild("AlbedoColor")) {
            const auto& colorArray = root.getChild("AlbedoColor").getArrayElements();
            if (colorArray.size() >= 4) {
                material->setAlbedoColor(Color(colorArray[0].getFloat(), colorArray[1].getFloat(), colorArray[2].getFloat(), colorArray[3].getFloat()));
            }
        }

        if (root.hasChild("Metallic")) {
            material->setMetallic(root.getChild("Metallic").getFloat());
        }

        if (root.hasChild("Roughness")) {
            material->setRoughness(root.getChild("Roughness").getFloat());
        }

        if (root.hasChild("Emission")) {
            material->setEmission(root.getChild("Emission").getFloat());
        }

        if (root.hasChild("UVTiling")) {
            const auto& uvArray = root.getChild("UVTiling").getArrayElements();
            if (uvArray.size() >= 2) {
                material->setUvTiling(Math::Vec2(uvArray[0].getFloat(), uvArray[1].getFloat()));
            }
        }

        if (root.hasChild("AlbedoMap")) {
            material->setAlbedoMap(UUID(std::stoull(root.getChild("AlbedoMap").getString())));
        }
        if (root.hasChild("NormalMap")) {
            material->setNormalMap(UUID(std::stoull(root.getChild("NormalMap").getString())));
        }
        if (root.hasChild("MetallicRoughnessMap")) {
            material->setMetallicRoughnessMap(UUID(std::stoull(root.getChild("MetallicRoughnessMap").getString())));
        }

        return material;
    }
} // namespace Axiom
