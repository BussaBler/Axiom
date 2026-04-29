#pragma once
#include "Components/ComponentReflection.h"
#include "Scene.h"
#include "Utils/FileSystem.h"
#include "Utils/JSONSerializer.h"

namespace Axiom {
    class SceneSerializer {
      public:
        SceneSerializer(Scene* scene) : scene(scene) {}
        ~SceneSerializer() = default;

        void serialize(const std::string& filePath);
        bool deserialize(const std::string& filePath);

      private:
        Scene* scene;
    };
} // namespace Axiom