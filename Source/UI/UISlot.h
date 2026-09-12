#pragma once

#include "Math/Vec.h"
#include "UI/UIEdgeInsets.h"
#include "UI/UIElement.h"

#include <memory>
#include <utility>

namespace Axiom {
    enum class UIAlignment { Fill, Start, Center, End };

    class UISlot {
      public:
        explicit UISlot(std::shared_ptr<UIElement> content) : content(std::move(content)) {}

        UISlot& setMargin(const UIEdgeInsets& newMargin) {
            margin = newMargin;
            return *this;
        }
        UISlot& setHorizontalAlignment(UIAlignment alignment) {
            horizontalAlignment = alignment;
            return *this;
        }
        UISlot& setVerticalAlignment(UIAlignment alignment) {
            verticalAlignment = alignment;
            return *this;
        }
        UISlot& setAlignment(UIAlignment horizontal, UIAlignment vertical) {
            horizontalAlignment = horizontal;
            verticalAlignment = vertical;
            return *this;
        }
        UISlot& setFixedSize(const Math::Vec2& size) {
            fixedSize = size;
            return *this;
        }

      public:
        std::shared_ptr<UIElement> content;

        UIEdgeInsets margin;
        UIAlignment horizontalAlignment = UIAlignment::Fill;
        UIAlignment verticalAlignment = UIAlignment::Fill;
        Math::Vec2 fixedSize = Math::Vec2(-1.0f);
    };
} // namespace Axiom
