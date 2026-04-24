#pragma once
#include "Asset.h"
#include "MeshAsset.h"
#include "ShaderAsset.h"
#include "TextureAsset.h"
#include "axpch.h"

namespace Axiom {
    class AssetManager {
        friend class Application;

      public:
        template <typename T> static std::shared_ptr<T> getAsset(UUID handle) {
            if (assets.find(handle) != assets.end()) {
                return std::static_pointer_cast<T>(assets[handle]);
            }
            return nullptr;
        }

        static UUID loadTexture(const std::filesystem::path& path);
        static UUID loadShader(const std::filesystem::path& path);
        static UUID loadMesh(const std::filesystem::path& path);

        static Buffer* getGlobalVertexBuffer() { return globalVertexBuffer.get(); }
        static Buffer* getGlobalIndexBuffer() { return globalIndexBuffer.get(); }

      private:
        static void init();
        static void shutdown();

      private:
        static std::unordered_map<UUID, std::shared_ptr<Asset>> assets;
        static std::unordered_map<std::string, UUID> assetHandles;

        static std::unique_ptr<Buffer> globalVertexBuffer;
        static std::unique_ptr<Buffer> globalIndexBuffer;
        static uint32_t currentVertexCount;
        static uint32_t currentIndexCount;
    };
} // namespace Axiom
