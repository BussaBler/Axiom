#pragma once
#include "Asset.h"
#include "Renderer/Shader.h"

namespace Axiom {
    class ShaderAsset : public Asset {
      public:
        ShaderAsset(UUID handle, const std::string& name, std::shared_ptr<Shader> shader) : Asset(handle, AssetType::Shader, name), shader(shader) {};
        ~ShaderAsset() = default;

        inline std::shared_ptr<Shader> getShader() const { return shader; }
        inline Shader* getShader() { return shader.get(); }

      private:
        std::shared_ptr<Shader> shader;
    };
}