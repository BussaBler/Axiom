#pragma once
#include "Registry.h"
#include "axpch.h"

namespace Axiom {
    class System {
      public:
        virtual ~System() = default;

        virtual void onUpdate(Registry* registry, float deltaTime) = 0;
    };

    class SystemManager {
      public:
        SystemManager() = default;
        ~SystemManager() = default;

        template <typename T> std::shared_ptr<T> registerSystem() {
            const char* typeName = typeid(T).name();
            AX_CORE_ASSERT(systems.find(typeName) == systems.end(), "Registering system more than once.");

            auto system = std::make_shared<T>();
            systems.insert({typeName, system});
            return system;
        }
        template <typename T> void setSignature(std::bitset<32> signature) {
            const char* typeName = typeid(T).name();
            AX_CORE_ASSERT(systems.find(typeName) != systems.end(), "System used before registered.");

            systemSignatureMap.insert({typeName, signature});
        }

      private:
        std::unordered_map<const char*, std::bitset<32>> systemSignatureMap;
        std::unordered_map<const char*, std::shared_ptr<System>> systems;
    };
} // namespace Axiom