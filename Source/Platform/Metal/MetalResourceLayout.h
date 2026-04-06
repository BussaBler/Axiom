#pragma once
#include "Renderer/ResourceLayout.h"

namespace Axiom {
    class MetalResourceLayout : public ResourceLayout {
      public:
        MetalResourceLayout(const std::vector<BindingCreateInfo>& bindingsCreateInfo);
        ~MetalResourceLayout() override = default;

        std::vector<BindingCreateInfo>& getBindingsCreateInfo() {
            return bindingsCreateInfo;
        }

      private:
        std::vector<BindingCreateInfo> bindingsCreateInfo;
    };
} // namespace Axiom