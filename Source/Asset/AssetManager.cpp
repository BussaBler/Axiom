#include "axpch.h"

#include "AssetManager.h"

#include "Asset/Asset.h"
#include "Asset/MaterialAsset.h"
#include "Asset/UUID.h"
#include "AxImageLoader.h"
#include "AxModelLoader.h"
#include "BuiltInShaders.h"
#include "Core/Log.h"
#include "Core/TaskManager.h"
#include "Math/Color.h"
#include "MeshAsset.h"
#include "Renderer/Buffer.h"
#include "Renderer/RenderHardwareInterface.h"
#include "Renderer/Shader.h"
#include "Renderer/ShaderCompiler.h"
#include "Renderer/Texture.h"
#include "ShaderAsset.h"
#include "TextureAsset.h"
#include "Utils/FileSystem.h"
#include "Utils/JSONSerializer.h"

#include <cstddef>
#include <cstdint>
#include <expected>
#include <memory>
#include <string>
#include <thread>
#include <utility>

namespace Axiom {
    AssetManager::AssetManager(RenderHardwareInterface& rhi, TaskManager& taskManager) : rhi(rhi), taskManager(taskManager) {
        uint64_t globalBufferSize = Math::megabytes(256);

        Buffer::CreateInfo vertexBufferCreateInfo = {
            .size = globalBufferSize, .usage = BufferUsage::Vertex | BufferUsage::TransferDst, .memoryUsage = MemoryUsage::GPUOnly};
        globalVertexBuffer = rhi.createBuffer(vertexBufferCreateInfo);
        Buffer::CreateInfo indexBufferCreateInfo = {
            .size = globalBufferSize, .usage = BufferUsage::Index | BufferUsage::TransferDst, .memoryUsage = MemoryUsage::GPUOnly};
        globalIndexBuffer = rhi.createBuffer(indexBufferCreateInfo);

        deserializeManifest("axiom://Packages/Packages.json");

        initDefaultAssets();
    }

    AssetManager::~AssetManager() {
        loadedAssets.clear();
        assetHandles.clear();

        globalVertexBuffer.reset();
        globalIndexBuffer.reset();

        serializeManifest("axiom://Packages/Packages.json", "axiom://");

        registry.clear();
    }

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
                                                 .memoryUsage = MemoryUsage::GPUOnly};
        std::unique_ptr<Texture> defaultTexture = rhi.createTexture(textureCreateInfo);

        Buffer::CreateInfo stagingBufferCreateInfo = {
            .size = defaultTextureSize * defaultTextureSize * sizeof(uint32_t), .usage = BufferUsage::TransferSrc, .memoryUsage = MemoryUsage::GPUandCPU};
        std::unique_ptr<Buffer> stagingBuffer = rhi.createBuffer(stagingBufferCreateInfo);

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

        std::unique_ptr<CommandBuffer> commandBuffer = rhi.beginSingleTimeCommands();
        stagingBuffer->setData<uint32_t>(defaultTextureData);
        commandBuffer->copyBufferToTexture(stagingBuffer.get(), defaultTexture.get(), defaultTextureSize, defaultTextureSize);
        rhi.endSingleTimeCommands(commandBuffer.get());

        AssetMetadata defaultTextureMeta = {.name = "BuiltIn.DefaultTexture", .type = AssetType::Texture, .filePath = ""};
        registry[DEFAULT_TEXTURE_HANDLE] = defaultTextureMeta;
        loadedAssets[DEFAULT_TEXTURE_HANDLE] = std::make_shared<TextureAsset>(DEFAULT_MATERIAL_HANDLE, "BuiltIn.DefaultTexture", std::move(defaultTexture));

        // Default material
        std::filesystem::path defaultShaderPath = "axiom://Packages/Shaders/BuiltIn.DefaultPBR.axs";
        UUID defaultShaderHandle = importAsset("BuiltIn.DefaultPBR", defaultShaderPath, AssetType::Shader);

        auto material = std::make_shared<MaterialAsset>(DEFAULT_MATERIAL_HANDLE, "BuiltIn.DefaultPBR", defaultShaderHandle, nullptr);
        material->setAlbedoColor(Color::lightGray());

        AssetMetadata defaultMaterialMeta = {.name = "BuiltIn.DefaultMaterial", .type = AssetType::Material, .filePath = ""};
        registry[DEFAULT_MATERIAL_HANDLE] = defaultMaterialMeta;
        loadedAssets[DEFAULT_MATERIAL_HANDLE] = material;

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
        std::unique_ptr<Buffer> vertexStaging = rhi.createBuffer(vertexStagingInfo);
        vertexStaging->setData(cubeVertices.data(), vertexBytes);

        Buffer::CreateInfo indexStagingInfo = {.size = indexBytes, .usage = BufferUsage::TransferSrc, .memoryUsage = MemoryUsage::GPUandCPU};
        std::unique_ptr<Buffer> indexStaging = rhi.createBuffer(indexStagingInfo);
        indexStaging->setData(cubeIndices.data(), indexBytes);

        auto cubeCommandBuffer = rhi.beginSingleTimeCommands();
        cubeCommandBuffer->copyBuffer(vertexStaging.get(), globalVertexBuffer.get(), vertexBytes, 0);
        cubeCommandBuffer->copyBuffer(indexStaging.get(), globalIndexBuffer.get(), indexBytes, 0);
        rhi.endSingleTimeCommands(cubeCommandBuffer.get());

        currentVertexCount += cubeVertices.size();
        currentIndexCount += cubeIndices.size();

        AssetMetadata defaultMeshMeta = {.name = "Default Mesh", .type = AssetType::Mesh, .filePath = ""};
        registry[DEFAULT_MESH_HANDLE] = defaultMeshMeta;

        loadedAssets[DEFAULT_MESH_HANDLE] = std::make_shared<MeshAsset>(DEFAULT_MESH_HANDLE, "Default Mesh", 0, 0, cubeIndices.size());

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

        ShaderCompiler compiler("");
        std::expected<ShaderSources, std::string> sources = ShaderCompiler::split(errorShaderSource);
        std::expected<CompiledShader, std::string> compiled = compiler.compile(sources.value(), "error");

        std::unique_ptr<Shader> errorShader = rhi.createShader(compiled->vertex, compiled->fragment);
        AssetMetadata errorShaderMeta = {.name = "Error Shader", .type = AssetType::Shader, .filePath = ""};
        registry[ERROR_SHADER_HANDLE] = errorShaderMeta;
        loadedAssets[ERROR_SHADER_HANDLE] = std::make_shared<ShaderAsset>(ERROR_SHADER_HANDLE, "Error Shader", std::move(errorShader));

        registerBuiltInShaders();
    }

    void AssetManager::registerBuiltInShaders() {
        for (const auto& entry : BuiltInShaders::Table) {
            UUID handle = UUID::fromName("BuiltIn." + std::string(entry.name));
            auto asset = std::make_shared<ShaderAsset>(handle, std::string(entry.name), rhi.createShader(entry.vertex, entry.fragment));
            asset->setState(AssetState::Loaded);
            loadedAssets[handle] = asset;
            builtinShaders[std::string(entry.name)] = handle;
        }
    }

    std::shared_ptr<Asset> AssetManager::loadTexture(const std::filesystem::path& path, UUID uuid) {
        std::shared_ptr<TextureAsset> textureAsset = std::make_shared<TextureAsset>(uuid, path.filename().string(), nullptr);
        textureAsset->setState(AssetState::Loading);
        loadedAssets[uuid] = textureAsset;

        std::thread([this, path, textureAsset]() {
            auto imageResult = AxImageLoader::loadImage(path, 4);
            if (!imageResult.has_value()) {
                AX_CORE_LOG_ERROR("Failed to load texture({}): {}", uint64_t(textureAsset->getHandle()), imageResult.error());
                textureAsset->setState(AssetState::Failed);
                return;
            }

            taskManager.submitToMain(
                [this, textureAsset, imageData = std::move(imageResult->data), width = imageResult->width, height = imageResult->height]() {
                    Texture::CreateInfo textureCreateInfo = {.width = width,
                                                             .height = height,
                                                             .mipLevels = 1,
                                                             .arrayLayers = 1,
                                                             .format = Format::R8G8B8A8Unorm,
                                                             .usage = TextureUsage::Sampled | TextureUsage::TransferDst,
                                                             .aspect = TextureAspect::Color,
                                                             .memoryUsage = MemoryUsage::GPUOnly};
                    std::unique_ptr<Texture> texture = rhi.createTexture(textureCreateInfo);

                    Buffer::CreateInfo stagingBufferCreateInfo = {
                        .size = imageData.size(), .usage = BufferUsage::TransferSrc, .memoryUsage = MemoryUsage::GPUandCPU};
                    std::unique_ptr<Buffer> stagingBuffer = rhi.createBuffer(stagingBufferCreateInfo);
                    stagingBuffer->setData(imageData.data(), imageData.size());

                    std::unique_ptr<CommandBuffer> commandBuffer = rhi.beginSingleTimeCommands();
                    commandBuffer->copyBufferToTexture(stagingBuffer.get(), texture.get(), width, height);
                    rhi.endSingleTimeCommands(commandBuffer.get());

                    textureAsset->updateData(std::move(texture));
                    textureAsset->setState(AssetState::Loaded);
                });
        }).detach();

        return loadedAssets[uuid];
    }

    std::shared_ptr<Asset> AssetManager::loadShader(const std::filesystem::path& path, UUID uuid) {
        loadedAssets[uuid] = getErrorShader();

        auto source = FileSystem::readFileStr(path);
        if (source.empty()) {
            AX_CORE_LOG_ERROR_ONCE("Failed to read shader file: {}", path.generic_string());
            return loadedAssets[uuid];
        }
        std::shared_ptr<ShaderAsset> shaderAsset = std::make_shared<ShaderAsset>(uuid, path.filename().string(), nullptr);
        shaderAsset->setState(AssetState::Loading);
        loadedAssets[uuid] = shaderAsset;

        std::thread([this, path, shaderAsset]() {
            auto source = FileSystem::readFileStr(path);
            if (source.empty()) {
                AX_CORE_LOG_ERROR("Failed to read shader file: {}", path.generic_string());
                shaderAsset->setState(AssetState::Failed);
                return;
            }

            ShaderCompiler compiler("axiom://Packages/Shaders/Includes");
            std::expected<ShaderSources, std::string> sources = ShaderCompiler::split(source);
            if (!sources.has_value()) {
                AX_CORE_LOG_ERROR("{}", sources.error());
                return;
            }
            std::expected<CompiledShader, std::string> compiled = compiler.compile(sources.value(), path.filename().string());
            if (!compiled.has_value()) {
                AX_CORE_LOG_ERROR("{}", compiled.error());
                return;
            }

            taskManager.submitToMain([this, shaderAsset, path, &compiled]() {
                std::unique_ptr<Shader> shader = rhi.createShader(compiled->vertex, compiled->fragment);
                if (!shader || !shader->isCompiled()) {
                    AX_CORE_LOG_ERROR("Shader {} was not compiled properly", path.generic_string());
                    shaderAsset->setState(AssetState::Failed);
                    return;
                }

                shaderAsset->updateData(std::move(shader));
                shaderAsset->setState(AssetState::Loaded);
            });
        }).detach();

        return loadedAssets[uuid];
    }

    std::shared_ptr<Asset> AssetManager::loadMesh(const std::filesystem::path& path, UUID uuid) {
        auto defaultMesh = getDefaultMesh();
        std::shared_ptr<MeshAsset> meshAsset = std::make_shared<MeshAsset>(uuid, path.filename().string(), defaultMesh->getVertexOffset(),
                                                                           defaultMesh->getIndexOffset(), defaultMesh->getIndexCount());
        meshAsset->setState(AssetState::Loading);
        loadedAssets[uuid] = meshAsset;

        std::thread([this, path, uuid, meshAsset]() {
            std::fstream test(path);

            auto modelResult = AxModelLoader::loadModel(FileSystem::resolvePath(path));
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

            taskManager.submitToMain([this, meshAsset, path, vertices = std::move(vertices), indices = std::move(modelResult->indices)]() {
                uint32_t vertexCount = vertices.size();
                uint32_t indexCount = indices.size();
                uint32_t vertexBytes = vertexCount * sizeof(MeshVertex);
                uint32_t indexBytes = indexCount * sizeof(uint32_t);

                Buffer::CreateInfo vertexStagingInfo = {.size = vertexBytes, .usage = BufferUsage::TransferSrc, .memoryUsage = MemoryUsage::GPUandCPU};
                std::unique_ptr<Buffer> vertexStaging = rhi.createBuffer(vertexStagingInfo);
                vertexStaging->setData(vertices.data(), vertexBytes);

                Buffer::CreateInfo indexStagingInfo = {.size = indexBytes, .usage = BufferUsage::TransferSrc, .memoryUsage = MemoryUsage::GPUandCPU};
                std::unique_ptr<Buffer> indexStaging = rhi.createBuffer(indexStagingInfo);
                indexStaging->setData(indices.data(), indexBytes);

                auto commandBuffer = rhi.beginSingleTimeCommands();
                uint32_t vertexByteDstOffset = currentVertexCount * sizeof(MeshVertex);
                uint32_t indexByteDstOffset = currentIndexCount * sizeof(uint32_t);
                commandBuffer->copyBuffer(vertexStaging.get(), globalVertexBuffer.get(), vertexBytes, 0, vertexByteDstOffset);
                commandBuffer->copyBuffer(indexStaging.get(), globalIndexBuffer.get(), indexBytes, 0, indexByteDstOffset);
                rhi.endSingleTimeCommands(commandBuffer.get());

                meshAsset->updateData(currentVertexCount, currentIndexCount, indexCount);
                meshAsset->setState(AssetState::Loaded);

                currentVertexCount += vertexCount;
                currentIndexCount += indexCount;
            });
        }).detach();

        return loadedAssets[uuid];
    }

    std::shared_ptr<Asset> AssetManager::loadMaterial(const std::filesystem::path& path, UUID uuid) {
        std::shared_ptr<MaterialAsset> materialAsset =
            std::make_shared<MaterialAsset>(uuid, path.filename().string(), getDefaultMaterial()->getShader(), nullptr);
        materialAsset->setState(AssetState::Loading);
        loadedAssets[uuid] = materialAsset;

        std::thread([this, path, materialAsset]() {
            std::string fileContent = FileSystem::readFileStr(path);
            if (fileContent.empty()) {
                AX_CORE_LOG_ERROR("Failed to read material file: {}", path.generic_string());
                materialAsset->setState(AssetState::Failed);
                return;
            }

            JSONValue root = JSONSerializer::deserialize(fileContent);
            if (root.getType() != JSONValueType::Object) {
                AX_CORE_LOG_ERROR("Invalid material file format: {}", path.generic_string());
                materialAsset->setState(AssetState::Failed);
                return;
            }

            taskManager.submitToMain([materialAsset, path, root = std::move(root)]() {
                if (root.hasChild("Name")) {
                    materialAsset->setName(root.getChild("Name").getString());
                }
                if (root.hasChild("Shader")) {
                    materialAsset->setShader(UUID(std::stoull(root.getChild("Shader").getString())));
                }

                if (root.hasChild("AlbedoColor")) {
                    const auto& colorArray = root.getChild("AlbedoColor").getArrayElements();
                    if (colorArray.size() >= 4) {
                        materialAsset->setAlbedoColor(
                            Color(colorArray[0].getFloat(), colorArray[1].getFloat(), colorArray[2].getFloat(), colorArray[3].getFloat()));
                    }
                }

                if (root.hasChild("Metallic")) {
                    materialAsset->setMetallic(root.getChild("Metallic").getFloat());
                }
                if (root.hasChild("Roughness")) {
                    materialAsset->setRoughness(root.getChild("Roughness").getFloat());
                }
                if (root.hasChild("Emission")) {
                    materialAsset->setEmission(root.getChild("Emission").getFloat());
                }

                if (root.hasChild("UVTiling")) {
                    const auto& uvArray = root.getChild("UVTiling").getArrayElements();
                    if (uvArray.size() >= 2)
                        materialAsset->setUvTiling(Math::Vec2(uvArray[0].getFloat(), uvArray[1].getFloat()));
                }

                if (root.hasChild("AlbedoMap")) {
                    materialAsset->setAlbedoMap(UUID(std::stoull(root.getChild("AlbedoMap").getString())));
                }
                if (root.hasChild("NormalMap")) {
                    materialAsset->setNormalMap(UUID(std::stoull(root.getChild("NormalMap").getString())));
                }
                if (root.hasChild("MetallicRoughnessMap")) {
                    materialAsset->setMetallicRoughnessMap(UUID(std::stoull(root.getChild("MetallicRoughnessMap").getString())));
                }

                materialAsset->setState(AssetState::Loaded);
            });
        }).detach();

        return loadedAssets[uuid];
    }
} // namespace Axiom
