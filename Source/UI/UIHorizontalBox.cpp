#include "axpch.h"

#include "UIHorizontalBox.h"

#include "Math/Vec.h"
#include "UI/UISlot.h"

namespace Axiom {
    Math::Vec2 UIHorizontalBox::getDesiredSize(const UIContext& context) {
        desiredSize = Math::Vec2::zero();

        for (const auto& slot : slots) {
            Math::Vec2 slotSize = slot.content->getDesiredSize(context);

            if (slot.fixedSize.x() > 0) {
                slotSize.x() = slot.fixedSize.x();
            }
            if (slot.fixedSize.y() > 0) {
                slotSize.y() = slot.fixedSize.y();
            }

            slotSize += Math::Vec2(slot.margin.left + slot.margin.right, slot.margin.top + slot.margin.bottom);
            desiredSize = Math::Vec2(desiredSize.x() + slotSize.x(), std::max(desiredSize.y(), slotSize.y()));
        }

        desiredSize += Math::Vec2(padding.left + padding.right, padding.top + padding.bottom);
        return desiredSize;
    }

    void UIHorizontalBox::arrange(const UIContext& context, const Math::Vec2& position, const Math::Vec2& size) {
        UIElement::arrange(context, position, size);

        float currentX = arrangedPosition.x() + padding.left;
        float startY = arrangedPosition.y() + padding.top;
        float avaibleHeight = arrangedSize.y() - padding.top - padding.bottom;

        for (const auto& slot : slots) {
            currentX += slot.margin.left;

            Math::Vec2 slotDesiredSize = slot.content->getDesiredSize(context);
            if (slot.fixedSize.x() > 0) {
                slotDesiredSize.x() = slot.fixedSize.x();
            }
            if (slot.fixedSize.y() > 0) {
                slotDesiredSize.y() = slot.fixedSize.y();
            }

            float finalY = startY;
            float finalHeight = avaibleHeight;

            switch (slot.verticalAlignment) {
            case UIAlignment::Fill:
                finalHeight = avaibleHeight - slot.margin.top - slot.margin.bottom;
                finalY = startY + slot.margin.top;
                break;
            case UIAlignment::Start:
                finalHeight = slotDesiredSize.y();
                finalY = startY + slot.margin.top;
                break;
            case UIAlignment::Center:
                finalHeight = slotDesiredSize.y();
                finalY = startY + (avaibleHeight - slotDesiredSize.y()) / 2.0f;
                break;
            case UIAlignment::End:
                finalHeight = slotDesiredSize.y();
                finalY = startY + avaibleHeight - slotDesiredSize.y() - slot.margin.bottom;
            }

            slot.content->arrange(context, Math::Vec2(currentX, finalY), Math::Vec2(slotDesiredSize.x(), finalHeight));
            currentX += slotDesiredSize.x() + slot.margin.right;
        }
    }
} // namespace Axiom
