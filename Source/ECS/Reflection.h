#pragma once

#include "Asset/Asset.h"
#include "Asset/UUID.h"
#include "ECS/Entity.h"
#include "Math/Color.h"
#include "Math/Vec.h"

#include <cstdint>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>
namespace Axiom {
    struct EnumValue {
        int64_t value = 0;
    };

    using FieldValue = std::variant<bool, int, float, std::string, Math::Vec2, Math::Vec3, Math::Vec4, Color, UUID, EnumValue>;

    enum class FieldType { Bool, Int, Float, String, Vec2, Vec3, Vec4, Color, AssetHandle, Enum };

    template <typename T> consteval FieldType fieldTypeOf() {
        if constexpr (std::is_enum_v<T>) {
            return FieldType::Enum;
        } else if constexpr (std::is_same_v<T, bool>) {
            return FieldType::Bool;
        } else if constexpr (std::is_same_v<T, int>) {
            return FieldType::Int;
        } else if constexpr (std::is_same_v<T, float>) {
            return FieldType::Float;
        } else if constexpr (std::is_same_v<T, std::string>) {
            return FieldType::String;
        } else if constexpr (std::is_same_v<T, Math::Vec2>) {
            return FieldType::Vec2;
        } else if constexpr (std::is_same_v<T, Math::Vec3>) {
            return FieldType::Vec3;
        } else if constexpr (std::is_same_v<T, Math::Vec4>) {
            return FieldType::Vec4;
        } else if constexpr (std::is_same_v<T, Color>) {
            return FieldType::Color;
        } else if constexpr (std::is_same_v<T, UUID>) {
            return FieldType::AssetHandle;
        } else {
            static_assert(sizeof(T) == 0, "Unsupported type in AX_PROPERTY");
        }
    }

    class Field {
      public:
        template <typename C, typename T> static Field make(std::string name, T C::* member) {
            Field f;
            f.fieldName = std::move(name);
            f.fieldType = fieldTypeOf<T>();
            f.accessor = std::make_shared<MemberAccessor<T, C>>(member);
            return f;
        }

        Field& asset(AssetType type) {
            assetType = type;
            return *this;
        }
        Field& options(std::vector<std::string> names) {
            enumNames = std::move(names);
            return *this;
        }

        FieldValue get(Entity entity) const { return accessor->get(entity); }
        template <typename T> T get(Entity entity) const { return std::get<T>(accessor->get(entity)); }
        void set(Entity entity, const FieldValue& value) const { accessor->set(entity, value); }

        const std::string& getName() const { return fieldName; }
        FieldType getType() const { return fieldType; }
        AssetType getAssetType() const { return assetType; }
        const std::vector<std::string>& getEnumOptions() const { return enumNames; }

      private:
        struct Accessor {
            virtual ~Accessor() = default;
            virtual FieldValue get(Entity entity) const = 0;
            virtual void set(Entity entity, const FieldValue& value) const = 0;
        };

        template <typename T, typename C> struct MemberAccessor final : Accessor {
            explicit MemberAccessor(T C::* member) : member(member) {}

            FieldValue get(Entity entity) const override {
                const T& v = entity.getComponent<C>().*member;
                if constexpr (std::is_enum_v<T>) {
                    return EnumValue(static_cast<int64_t>(std::to_underlying(v)));
                } else {
                    return v;
                }
            }
            void set(Entity entity, const FieldValue& value) const override {
                T& v = entity.getComponent<C>().*member;
                if constexpr (std::is_enum_v<T>) {
                    v = static_cast<T>(std::get<EnumValue>(value).value);
                } else {
                    v = std::get<T>(value);
                }
            }

            T C::* member;
        };

      private:
        std::string fieldName;
        FieldType fieldType{};
        AssetType assetType = AssetType::None;
        std::vector<std::string> enumNames;
        std::shared_ptr<const Accessor> accessor;
    };

    class ComponentType {
      public:
        virtual ~ComponentType() = default;
        virtual void registerStorage(ECS& ecs) const = 0;
        virtual bool has(Entity entity) const = 0;
        virtual void add(Entity entity) const = 0;
        virtual void remove(Entity entity) const = 0;

        const std::string& getName() const { return typeName; }
        const std::vector<Field>& getFields() const { return typeFields; }

      protected:
        std::string typeName;
        std::vector<Field> typeFields;
    };

    class ComponentRegistry {
      public:
        void registerReflectedComponents();

        template <typename C> void add(std::string name, std::vector<Field> fields) {
            components.push_back(std::make_unique<RegistryComponentType<C>>(name, std::move(fields)));
        }

        const ComponentType* find(const std::string& name) const {
            for (const auto& comp : components) {
                if (comp->getName() == name) {
                    return comp.get();
                }
            }
            return nullptr;
        }
        const std::vector<std::unique_ptr<ComponentType>>& all() const { return components; }

      private:
        template <typename C> struct RegistryComponentType final : ComponentType {
            RegistryComponentType(std::string name, std::vector<Field> fields) {
                typeName = std::move(name);
                typeFields = std::move(fields);
            }

            void registerStorage(ECS& ecs) const override { ecs.registerComponent<C>(); }
            bool has(Entity entity) const override { return entity.hasComponent<C>(); }
            void add(Entity entity) const override { entity.addComponent<C>(C{}); }
            void remove(Entity entity) const override { entity.removeComponent<C>(); }
        };

      private:
        std::vector<std::unique_ptr<ComponentType>> components;
    };
} // namespace Axiom
