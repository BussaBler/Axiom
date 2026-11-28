#include "axpch.h"

#include "UIElement.h"

#include "Math/Vec.h"
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

    bool UIElement::containsPoint(Math::Vec2 point) const {
        if (point.x() >= arrangedPosition.x() && point.x() <= arrangedPosition.x() + arrangedSize.x() && point.y() >= arrangedPosition.y() &&
            point.y() <= arrangedPosition.y() + arrangedSize.y()) {
            return true;
        }
        return false;
    }
} // namespace Axiom
