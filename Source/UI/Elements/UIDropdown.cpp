#include "axpch.h"

#include "UIDropdown.h"

namespace Axiom {
    Math::Vec2 UIDropdown::getDesiredSize(const UIContext& context) {
        return Math::Vec2();
    }

    void UIDropdown::arrange(const UIContext& context, const Math::Vec2& position, const Math::Vec2& size) {
    }

    void UIDropdown::onRender(const UIContext& context, const Math::Rect& scissorRect) {
    }

    bool UIDropdown::onEvent(Event& event) {
        return false;
    }
} // namespace Axiom