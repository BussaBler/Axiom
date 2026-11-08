#pragma once
#include "Asset.h"
#include "Asset/MaterialAsset.h"
#include "Asset/MeshAsset.h"
#include "Asset/ShaderAsset.h"
#include "Asset/UUID.h"
#include "Core/Log.h"
#include "Core/TaskManager.h"
#include "Renderer/Buffer.h"
#include "Renderer/RenderHardwareInterface.h"
#include "TextureAsset.h"

#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>

namespace Axiom {
    struct AssetMetadata {
        std::string name = "";
        AssetType type = AssetType::None;
        std::filesystem::path filePath = "";
    };

    class AssetManager {
        friend class Application;

      public:
        AssetManager(RenderHardwareInterface& rhi, TaskManager& taskManager);
        ~AssetManager();

        template <typename T> std::shared_ptr<T> getAsset(UUID handle) {
            if (!handle.isValid()) {
                return nullptr;
            }

            if (loadedAssets.find(handle) != loadedAssets.end()) {
                return std::static_pointer_cast<T>(loadedAssets[handle]);
            }

            if (registry.find(handle) == registry.end()) {
                AX_CORE_LOG_ERROR_ONCE("Tried to load an asset that is not registered: {}", handle);
                return nullptr;
            }

            const AssetMetadata& meta = registry[handle];
            std::shared_ptr<Asset> newAsset = nullptr;

            switch (meta.type) {
            case AssetType::Texture:
                newAsset = loadTexture(meta.filePath, handle);
                break;
            case AssetType::Shader:
                newAsset = loadShader(meta.filePath, handle);
                break;
            case AssetType::Mesh:
                newAsset = loadMesh(meta.filePath, handle);
                break;
            case AssetType::Material:
                newAsset = loadMaterial(meta.filePath, handle);
            default:
                break;
            }

            if (newAsset) {
                loadedAssets[handle] = newAsset;
                return std::static_pointer_cast<T>(newAsset);
            }

            return nullptr;
        }

        UUID importAsset(const std::string& name, const std::filesystem::path& path, AssetType type);

        Buffer* getGlobalVertexBuffer() { return globalVertexBuffer.get(); }
        Buffer* getGlobalIndexBuffer() { return globalIndexBuffer.get(); }

        const AssetMetadata& getMetadata(UUID handle) {
            static AssetMetadata nullMeta;
            if (!handle.isValid() || registry.find(handle) == registry.end()) {
                return nullMeta;
            }
            return registry.at(handle);
        }
        bool isAssetRegistered(UUID handle) {
            if (!handle.isValid()) {
                return false;
            }

            return registry.find(handle) != registry.end();
        }

        std::vector<UUID> getAssetsByType(AssetType type) {
            std::vector<UUID> handles;
            for (const auto& [handle, meta] : registry) {
                if (meta.type == type) {
                    handles.push_back(handle);
                }
            }
            return handles;
        }

        inline UUID getDefaultTextureHandle() { return DEFAULT_TEXTURE_HANDLE; }
        inline std::shared_ptr<TextureAsset> getDefaultTexture() { return std::static_pointer_cast<TextureAsset>(loadedAssets[DEFAULT_TEXTURE_HANDLE]); }
        inline std::shared_ptr<MaterialAsset> getDefaultMaterial() { return std::static_pointer_cast<MaterialAsset>(loadedAssets[DEFAULT_MATERIAL_HANDLE]); }
        inline std::shared_ptr<MeshAsset> getDefaultMesh() { return std::static_pointer_cast<MeshAsset>(loadedAssets[DEFAULT_MESH_HANDLE]); }
        inline std::shared_ptr<ShaderAsset> getErrorShader() { return std::static_pointer_cast<ShaderAsset>(loadedAssets[ERROR_SHADER_HANDLE]); }
        inline std::shared_ptr<ShaderAsset> getBuiltInShader(const std::string& name) {
            auto it = builtinShaders.find(name);
            if (it == builtinShaders.end()) {
                AX_CORE_LOG_ERROR("{} is not a BuiltIn Shader", name);
                return nullptr;
            }
            return std::static_pointer_cast<ShaderAsset>(loadedAssets[builtinShaders[name]]);
        }

        void serializeManifest(const std::filesystem::path& manifestPath, const std::string& vfsFilter);
        void deserializeManifest(const std::filesystem::path& manisfestPath);

      private:
        void initDefaultAssets();
        void registerBuiltInShaders();

        std::shared_ptr<Asset> loadTexture(const std::filesystem::path& path, UUID uuid);
        std::shared_ptr<Asset> loadShader(const std::filesystem::path& path, UUID uuid);
        std::shared_ptr<Asset> loadMesh(const std::filesystem::path& path, UUID uuid);
        std::shared_ptr<Asset> loadMaterial(const std::filesystem::path& path, UUID uuid);

      private:
        RenderHardwareInterface& rhi;
        TaskManager& taskManager;
        std::unordered_map<UUID, AssetMetadata> registry;
        std::unordered_map<UUID, std::shared_ptr<Asset>> loadedAssets;
        std::unordered_map<std::string, UUID> assetHandles;
        std::unordered_map<std::string, UUID> builtinShaders;

        std::unique_ptr<Buffer> globalVertexBuffer;
        std::unique_ptr<Buffer> globalIndexBuffer;
        uint32_t currentVertexCount = 0;
        uint32_t currentIndexCount = 0;

        inline static const UUID DEFAULT_TEXTURE_HANDLE = 0;
        inline static const UUID DEFAULT_MATERIAL_HANDLE = 1;
        inline static const UUID DEFAULT_MESH_HANDLE = 2;
        inline static const UUID ERROR_SHADER_HANDLE = 3;
    };
} // namespace Axiom
