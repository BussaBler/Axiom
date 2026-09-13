#pragma once
#include "Scene.h"

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
