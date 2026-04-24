#include "Asset.h"
#include "Renderer/Texture.h"
#include "axpch.h"

namespace Axiom {
    class TextureAsset : public Asset {
      public:
        TextureAsset(UUID handle, const std::string& name, std::unique_ptr<Texture> texture)
            : Asset(handle, AssetType::Texture, name), texture(std::move(texture)) {}
        ~TextureAsset() = default;

        inline Texture* getTexture() const { return texture.get(); }

      private:
        std::unique_ptr<Texture> texture;
    };
} // namespace Axiom