#pragma once
#include "Asset.h"
#include "Asset/MaterialAsset.h"
#include "Asset/MeshAsset.h"
#include "Asset/ShaderAsset.h"
#include "Asset/UUID.h"
#include "Core/Locator.h"
#include "Core/Log.h"
#include "Renderer/Buffer.h"
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
        template <typename T> static std::shared_ptr<T> getAsset(UUID handle) {
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

        static UUID importAsset(const std::string& name, const std::filesystem::path& path, AssetType type);

        static Buffer* getGlobalVertexBuffer() { return globalVertexBuffer.get(); }
        static Buffer* getGlobalIndexBuffer() { return globalIndexBuffer.get(); }

        static const AssetMetadata& getMetadata(UUID handle) {
            static AssetMetadata nullMeta;
            if (!handle.isValid() || registry.find(handle) == registry.end()) {
                return nullMeta;
            }
            return registry.at(handle);
        }
        static bool isAssetRegistered(UUID handle) {
            if (!handle.isValid()) {
                return false;
            }

            return registry.find(handle) != registry.end();
        }

        static std::vector<UUID> getAssetsByType(AssetType type) {
            std::vector<UUID> handles;
            for (const auto& [handle, meta] : registry) {
                if (meta.type == type) {
                    handles.push_back(handle);
                }
            }
            return handles;
        }

        inline static UUID getDefaultTextureHandle() { return defaultTextureHandle; }
        inline static std::shared_ptr<TextureAsset> getDefaultTexture() { return std::static_pointer_cast<TextureAsset>(loadedAssets[defaultTextureHandle]); }
        inline static std::shared_ptr<MaterialAsset> getDefaultMaterial() {
            return std::static_pointer_cast<MaterialAsset>(loadedAssets[defaultMaterialHandle]);
        }
        inline static std::shared_ptr<MeshAsset> getDefaultMesh() { return std::static_pointer_cast<MeshAsset>(loadedAssets[defaultMeshHandle]); }
        inline static std::shared_ptr<ShaderAsset> getErrorShader() { return std::static_pointer_cast<ShaderAsset>(loadedAssets[errorShaderHandle]); }

        static void serializeManifest(const std::filesystem::path& manifestPath, const std::string& vfsFilter);
        static void deserializeManifest(const std::filesystem::path& manisfestPath);

      private:
        static void init();
        static void shutdown();

        static void initDefaultAssets();

        static std::shared_ptr<Asset> loadTexture(const std::filesystem::path& path, UUID uuid);
        static std::shared_ptr<Asset> loadShader(const std::filesystem::path& path, UUID uuid);
        static std::shared_ptr<Asset> loadMesh(const std::filesystem::path& path, UUID uuid);
        static std::shared_ptr<Asset> loadMaterial(const std::filesystem::path& path, UUID uuid);

      private:
        static std::unordered_map<UUID, AssetMetadata> registry;
        static std::unordered_map<UUID, std::shared_ptr<Asset>> loadedAssets;
        static std::unordered_map<std::string, UUID> assetHandles;

        static std::unique_ptr<Buffer> globalVertexBuffer;
        static std::unique_ptr<Buffer> globalIndexBuffer;
        static uint32_t currentVertexCount;
        static uint32_t currentIndexCount;

        inline static const UUID defaultTextureHandle = 0;
        inline static const UUID defaultMaterialHandle = 1;
        inline static const UUID defaultMeshHandle = 2;
        inline static const UUID errorShaderHandle = 3;
    };
} // namespace Axiom
