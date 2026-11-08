#pragma once

#include "ECS/ECS.h"

#include <functional>
#include <vector>
namespace Axiom {
    class ComponentRegistry {
      public:
        ComponentRegistry() = default;
        ~ComponentRegistry() = default;
        template <typename T> void registerComponent() {
            registerFunctions.push_back([](ECS* ecs) { ecs->registerComponent<T>(); });
        }
        void registerDefaultComponents();

        const std::vector<std::function<void(ECS*)>>& getRegisterFunctions() { return registerFunctions; }

      private:
        std::vector<std::function<void(ECS*)>> registerFunctions;
    };
} // namespace Axiom
