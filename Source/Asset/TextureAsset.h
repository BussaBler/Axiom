#include "Asset.h"
#include "Renderer/Texture.h"
#include "axpch.h"

namespace Axiom {
    class TextureAsset : public Asset {
      public:
        TextureAsset(UUID handle, const std::string& name, std::shared_ptr<Texture> texture) : Asset(handle, AssetType::Texture, name), texture(texture) {}
        ~TextureAsset() = default;

        inline std::shared_ptr<Texture> getTexture() const { return texture; }

      private:
        std::shared_ptr<Texture> texture;
    };
} // namespace Axiom