#include "axpch.h"

#include "UI/UIVerticalBox.h"

#include "Math/Vec.h"

namespace Axiom {
    Math::Vec2 UIVerticalBox::getDesiredSize(const UIContext& context) {
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
            desiredSize = Math::Vec2(std::max(desiredSize.x(), slotSize.x()), desiredSize.y() + slotSize.y());
        }

        desiredSize += Math::Vec2(padding.left + padding.right, padding.top + padding.bottom);
        return desiredSize;
    }

    void UIVerticalBox::arrange(const UIContext& context, const Math::Vec2& position, const Math::Vec2& size) {
        UIElement::arrange(context, position, size);

        float currentY = arrangedPosition.y() + padding.top;
        float startX = arrangedPosition.x() + padding.left;
        float availableWidth = arrangedSize.x() - padding.left - padding.right;

        for (auto& slot : slots) {
            currentY += slot.margin.top;

            Math::Vec2 slotDesiredSize = slot.content->getDesiredSize(context);
            if (slot.fixedSize.x() > 0) {
                slotDesiredSize.x() = slot.fixedSize.x();
            }
            if (slot.fixedSize.y() > 0) {
                slotDesiredSize.y() = slot.fixedSize.y();
            }

            float finalX = startX;
            float finalWidth = availableWidth;

            switch (slot.horizontalAlignment) {
            case UIAlignment::Fill:
                finalWidth = availableWidth - slot.margin.left - slot.margin.right;
                finalX = startX + slot.margin.left;
                break;
            case UIAlignment::Start:
                finalWidth = slotDesiredSize.x();
                finalX = startX + slot.margin.left;
                break;
            case UIAlignment::Center:
                finalWidth = slotDesiredSize.x();
                finalX = startX + (availableWidth - slotDesiredSize.x()) / 2.0f;
                break;
            case UIAlignment::End:
                finalWidth = slotDesiredSize.x();
                finalX = startX + availableWidth - slotDesiredSize.x() - slot.margin.right;
                break;
            }

            slot.content->arrange(context, Math::Vec2(finalX, currentY), Math::Vec2(finalWidth, slotDesiredSize.y()));
            currentY += slotDesiredSize.y() + slot.margin.bottom;
        }
    }
} // namespace Axiom
