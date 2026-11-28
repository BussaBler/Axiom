#include "axpch.h"

#include "MetalResourceLayout.h"

#include <algorithm>
#include <cstdint>

namespace Axiom {
    MetalResourceLayout::MetalResourceLayout(const std::vector<BindingCreateInfo>& bindingsCreateInfo) : ResourceLayout() {
        this->bindingsCreateInfo = bindingsCreateInfo;
        std::sort(this->bindingsCreateInfo.begin(), this->bindingsCreateInfo.end(),
                  [](const BindingCreateInfo a, const BindingCreateInfo b) { return a.binding < b.binding; });

        uint32_t next = 0;
        for (const auto& binding : this->bindingsCreateInfo) {
            firstIndex[binding.binding] = next;
            next += binding.count;
        }
    }
} // namespace Axiom
