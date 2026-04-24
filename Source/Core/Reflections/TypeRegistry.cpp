#pragma once
#include "TypeRegistry.h"

namespace Axiom {
    std::unordered_map<std::type_index, std::vector<FieldInfo>> TypeRegistry::registry;
} // namespace Axiom