#pragma once
#include "Renderer/ResourceLayout.h"

#include <vector>

namespace Axiom {
    class MetalResourceLayout : public ResourceLayout {
      public:
        MetalResourceLayout(const std::vector<BindingCreateInfo>& bindingsCreateInfo);
        ~MetalResourceLayout() override = default;

        const std::vector<BindingCreateInfo>& getBindingsCreateInfo() const { return bindingsCreateInfo; }
        uint32_t getFirstIndex(uint32_t binding) const { return firstIndex.at(binding); }
        uint32_t getCount(uint32_t binding) const { return bindingsCreateInfo[binding].count; }

      private:
        std::vector<BindingCreateInfo> bindingsCreateInfo;
        std::unordered_map<uint32_t, uint32_t> firstIndex;
    };
} // namespace Axiom
