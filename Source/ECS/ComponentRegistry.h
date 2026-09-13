#pragma once

#include "ECS/ECS.h"

#include <functional>
#include <vector>
namespace Axiom {
    class ComponentRegistry {
      public:
        template <typename T> static void registerComponent() {
            registerFunctions.push_back([](ECS* ecs) { ecs->registerComponent<T>(); });
        }
        static void registerDefaultComponents();

        static const std::vector<std::function<void(ECS*)>>& getRegisterFunctions() { return registerFunctions; }

      private:
        inline static std::vector<std::function<void(ECS*)>> registerFunctions = {};
    };
} // namespace Axiom
