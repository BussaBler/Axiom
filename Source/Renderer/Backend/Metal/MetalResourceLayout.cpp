#include "MetalResourceLayout.h"

namespace Axiom {
    MetalResourceLayout::MetalResourceLayout(const std::vector<BindingCreateInfo>& bindingsCreateInfo) : ResourceLayout() {
        this->bindingsCreateInfo = bindingsCreateInfo;
    }
}