#include "axpch.h"

#include "SceneSerializer.h"

#include "Core/Log.h"
#include "ECS/Components/TagComponent.h"
#include "ECS/Entity.h"
#include "ECS/Reflection.h"
#include "Math/Color.h"
#include "Utils/FileSystem.h"
#include "Utils/JSONValue.h"
#include "Utils/Serializer.h"

#include <charconv>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <string>
#include <utility>
#include <variant>

namespace Axiom {
    namespace {
        std::optional<UUID> readUUID(const JSONValue& json) {
            auto text = json.text();
            if (!text) {
                return std::nullopt;
            }
            uint64_t value = 0;
            const char* first = text->data();
            const char* last = first + text->size();
            auto [ptr, ec] = std::from_chars(first, last, value);
            if (ec != std::errc() || ptr != last) {
                return std::nullopt;
            }
            return UUID(value);
        }

        JSONValue toJSON(const Field& field, const FieldValue& value) {
            return std::visit(Overloaded{
                                  [](bool v) -> JSONValue { return v; },
                                  [](int v) -> JSONValue { return v; },
                                  [](float v) -> JSONValue { return static_cast<double>(v); },
                                  [](const std::string& v) -> JSONValue { return v; },
                                  [](const Math::Vec2& v) -> JSONValue { return JSONValue::Array{double(v.x()), double(v.y())}; },
                                  [](const Math::Vec3& v) -> JSONValue { return JSONValue::Array{double(v.x()), double(v.y()), double(v.z())}; },
                                  [](const Math::Vec4& v) -> JSONValue { return JSONValue::Array{double(v.x()), double(v.y()), double(v.z()), double(v.w())}; },
                                  [](const Color& c) -> JSONValue { return JSONValue::Array{double(c.r()), double(c.g()), double(c.b()), double(c.a())}; },
                                  [](const UUID& id) -> JSONValue { return std::to_string(static_cast<uint64_t>(id)); },
                                  [&field](const EnumValue& e) -> JSONValue {
                                      const auto& names = field.getEnumOptions();
                                      if (e.value >= 0 && static_cast<size_t>(e.value) < names.size()) {
                                          return names[e.value];
                                      }
                                      return e.value;
                                  },
                              },
                              value);
        }

        template <typename T> std::optional<FieldValue> makeValue(T value) {
            return FieldValue(std::in_place_type<T>, std::move(value));
        }

        std::optional<FieldValue> fromJSON(const Field& field, const JSONValue& json) {
            switch (field.getType()) {
            case FieldType::Bool:
                if (json.is<bool>())
                    return makeValue(json.get<bool>());
                return std::nullopt;

            case FieldType::Int:
                if (json.is<int64_t>())
                    return makeValue(static_cast<int>(json.get<int64_t>()));
                return std::nullopt;

            case FieldType::Float:
                if (auto n = json.number())
                    return makeValue(static_cast<float>(*n));
                return std::nullopt;

            case FieldType::String:
                if (auto s = json.text())
                    return makeValue(std::move(*s));
                return std::nullopt;

            case FieldType::Vec2:
                if (auto v = json.floats<2>())
                    return makeValue(Math::Vec2((*v)[0], (*v)[1]));
                return std::nullopt;

            case FieldType::Vec3:
                if (auto v = json.floats<3>())
                    return makeValue(Math::Vec3((*v)[0], (*v)[1], (*v)[2]));
                return std::nullopt;

            case FieldType::Vec4:
                if (auto v = json.floats<4>())
                    return makeValue(Math::Vec4((*v)[0], (*v)[1], (*v)[2], (*v)[3]));
                return std::nullopt;

            case FieldType::Color:
                if (auto v = json.floats<4>())
                    return makeValue(Color((*v)[0], (*v)[1], (*v)[2], (*v)[3]));
                return std::nullopt;

            case FieldType::AssetHandle:
                if (auto id = readUUID(json))
                    return makeValue(*id);
                return std::nullopt;

            case FieldType::Enum: {
                const auto& names = field.getEnumOptions();
                if (auto name = json.text()) {
                    for (size_t i = 0; i < names.size(); i++) {
                        if (names[i] == *name) {
                            return makeValue(EnumValue{static_cast<int64_t>(i)});
                        }
                    }
                    return std::nullopt;
                }
                if (json.is<int64_t>()) {
                    return makeValue(EnumValue{json.get<int64_t>()});
                }
                return std::nullopt;
            }
            }
            return std::nullopt;
        }
    } // namespace

    void SceneSerializer::serialize(const std::string& filePath) {
        JSONValue root;
        JSONValue& entitiesArray = root["Entities"];
        entitiesArray = JSONValue::Array{};

        for (auto entityId : scene->ecs->view<TagComponent>()) {
            Entity entity = Entity(entityId, scene->ecs.get());

            JSONValue entityNode = JSONValue::Objects{};
            JSONValue& componentsNode = entityNode["Components"];
            componentsNode = JSONValue::Objects{};

            for (const auto& type : scene->componentRegistry.all()) {
                if (!type->has(entity)) {
                    continue;
                }

                JSONValue componentNode = JSONValue::Objects{};
                for (const Field& field : type->getFields()) {
                    componentNode[field.getName()] = toJSON(field, field.get(entity));
                }
                componentsNode[type->getName()] = std::move(componentNode);
            }

            entitiesArray.append(std::move(entityNode));
        }

        FileSystem::writeFile(filePath, Serializer::serializeJSON(root, 2));
    }

    bool SceneSerializer::deserialize(const std::string& filePath) {
        std::string fileContent = FileSystem::readFileStr(filePath);
        if (fileContent.empty()) {
            AX_CORE_LOG_ERROR("Failed to read scene file: {}", filePath);
            return false;
        }

        auto json = Serializer::deserializeJSON(fileContent);
        if (!json) {
            AX_CORE_LOG_ERROR("Invalid scene file {}: {}", filePath, json.error());
            return false;
        }

        const JSONValue& entitiesArray = (*json)["Entities"];
        if (!entitiesArray.is<JSONValue::Array>()) {
            AX_CORE_LOG_ERROR("Invalid scene file {}: no 'Entities' array", filePath);
            return false;
        }

        for (const JSONValue& entityJson : entitiesArray.get<JSONValue::Array>()) {
            const JSONValue& components = entityJson["Components"];
            if (!components.is<JSONValue::Objects>()) {
                AX_CORE_LOG_WARN("Skipping entity without a 'Components' object in {}", filePath);
                continue;
            }

            Entity entity = scene->newEntity();

            for (const auto& [componentName, componentJson] : components.get<JSONValue::Objects>()) {
                const ComponentType* type = scene->componentRegistry.find(componentName);
                if (!type) {
                    AX_CORE_LOG_WARN("Unknown component '{}' in {}, skipping it", componentName, filePath);
                    continue;
                }

                type->add(entity);
                for (const Field& field : type->getFields()) {
                    if (auto value = fromJSON(field, componentJson[field.getName()])) {
                        field.set(entity, *value);
                    }
                }
            }
        }

        return true;
    }
} // namespace Axiom
