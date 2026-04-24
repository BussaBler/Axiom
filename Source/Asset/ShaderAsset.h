#pragma once
#include "Asset.h"
#include "Renderer/Shader.h"

namespace Axiom {
    class ShaderAsset : public Asset {
      public:
        ShaderAsset(UUID handle, const std::string& name, std::unique_ptr<Shader> shader) : Asset(handle, AssetType::Shader, name), shader(std::move(shader)) {}
        ~ShaderAsset() = default;

        inline Shader* getShader() const { return shader.get(); }

      private:
        std::unique_ptr<Shader> shader;
    };
}