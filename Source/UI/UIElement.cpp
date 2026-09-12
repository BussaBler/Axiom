#include "axpch.h"

#include "UIElement.h"

#include "UICanvas.h"

namespace Axiom {
    UICanvas* UIElement::getParentCanvas() {
        UIElement* current = this;
        while (current) {
            if (current->isCanvas()) {
                return static_cast<UICanvas*>(current);
            }
            current = current->parent;
        }
        return nullptr;
    }
} // namespace Axiom
