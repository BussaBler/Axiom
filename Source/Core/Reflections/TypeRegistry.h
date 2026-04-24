#pragma once
#include "axpch.h"

namespace Axiom {
    enum class FieldType { Float, Int, Vec2, Vec3, Vec4, Color, String, Bool, AssetHandle };

    struct FieldInfo {
        std::string name;
        FieldType type;
        size_t offset;
    };

    class TypeRegistry {
      public:
        static void registerField(const std::type_index& type, const std::string& fieldName, FieldType fieldType, size_t offset) {
            getRegistry()[type].push_back({fieldName, fieldType, offset});
        }
        inline static const std::vector<FieldInfo>& getFields(const std::type_index& type) { return getRegistry()[type]; }

      private:
        static std::unordered_map<std::type_index, std::vector<FieldInfo>>& getRegistry() {
            static std::unordered_map<std::type_index, std::vector<FieldInfo>> registry;
            return registry;
        }

      private:
        static std::unordered_map<std::type_index, std::vector<FieldInfo>> registry;
    };

#define AX_REGISTER_FIELD(type, field, fieldType)                                                                                                              \
    inline const bool _##type##_##field##_registered = []() {                                                                                                  \
        TypeRegistry::registerField(typeid(type), #field, fieldType, offsetof(type, field));                                                                   \
        return true;                                                                                                                                           \
    }();
} // namespace Axiom