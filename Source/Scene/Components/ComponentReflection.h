#pragma once
#include "Core/Log.h"
#include "ECS/Entity.h"
#include "axpch.h"

namespace Axiom {
    enum class FieldType {
        None,
        Float,
        Int,
        Bool,
        String,
        Vec2,
        Vec3,
        Vec4,
        Color,
        AssetHandle,
    };

    struct FieldInfo {
        std::string name;
        FieldType type;
        size_t offset;
    };

    struct ComponentInfo {
        std::string name;
        size_t size;
        std::vector<FieldInfo> fields;
    };

    class ComponentReflection {
      public:
        static void init();
        inline static const ComponentInfo* getComponentInfo(const std::type_index& componentType) {
            auto it = componentRegistry.find(componentType);
            if (it != componentRegistry.end()) {
                return &it->second;
            } else {
                AX_LOG_ERROR("Component '{}' not found in reflection registry", componentType.name());
                return nullptr;
            }
        }
        inline static const ComponentInfo* getComponentInfo(const std::string& componentName) {
            for (const auto& [typeIndex, info] : componentRegistry) {
                if (info.name == componentName) {
                    return &info;
                }
            }
            return nullptr;
        }
        static void addComponent(Entity entity, const std::string& componentName, void* componentData);

      private:
        static std::unordered_map<std::type_index, ComponentInfo> componentRegistry;
    };
} // namespace Axiom