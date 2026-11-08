#pragma once
#include "UUID.h"

#include <string>

namespace Axiom {
    enum class AssetType { None, Texture, Mesh, Font, Audio, Shader, Material };
    enum class AssetState { Unloaded, Loading, Loaded, Failed };

    class Asset {
      public:
        Asset(UUID handle, AssetType type, const std::string& name) : handle(handle), type(type), name(name) {}
        virtual ~Asset() = default;

        inline UUID getHandle() const { return handle; }
        inline AssetType getType() const { return type; }
        inline AssetState getState() const { return state; }
        inline const std::string& getName() const { return name; }

        inline void setName(const std::string& newName) { name = newName; }
        inline void setState(AssetState newState) { state = newState; }

      protected:
        UUID handle = 0;
        AssetType type = AssetType::None;
        AssetState state = AssetState::Unloaded;
        std::string name;
    };
} // namespace Axiom
