#include "AssetManager.h"
#include "AxImageLoader.h"
#include "Core/Application.h"

namespace Axiom {
    UUID AssetManager::loadTexture(const std::filesystem::path& path) {
        if (assetHandles.find(path.string()) != assetHandles.end()) {
            return assetHandles[path.string()];
        }

        UUID handle = UUID();

        auto imageResult = AxImageLoader::loadImage(path);

        if (imageResult.has_value()) {
            // TODO: Create texture from image data and store it in the assets map
            return handle;
        }

        AX_CORE_LOG_ERROR("Failed to load texture: {}", imageResult.error());

        return 0;
    }
} // namespace Axiom
