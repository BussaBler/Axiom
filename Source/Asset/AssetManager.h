#pragma once
#include "Asset.h"
#include "axpch.h"

namespace Axiom {
    class AssetManager {
      public:
        template <typename T> static std::shared_ptr<T> getAsset(UUID handle) {
            if (assets.find(handle) != assets.end()) {
                return std::static_pointer_cast<T>(assets[handle]);
            }
            return nullptr;
        }

        static UUID loadTexture(const std::filesystem::path& path);

      private:
        static std::unordered_map<UUID, std::shared_ptr<Asset>> assets;
        static std::unordered_map<std::string, UUID> assetHandles;
    };
} // namespace Axiom
