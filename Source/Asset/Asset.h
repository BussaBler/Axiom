#pragma once
#include "UUID.h"
#include "axpch.h"

namespace Axiom {
    enum class AssetType { None, Texture, Mesh, Font, Audio };

    class Asset {
      public:
        Asset(UUID handle, AssetType type, const std::string &name) : handle(handle), type(type), name(name) {
        }
        ~Asset() = default;

      private:
        UUID handle = 0;
        AssetType type = AssetType::None;
        const std::string name;
    };
} // namespace Axiom