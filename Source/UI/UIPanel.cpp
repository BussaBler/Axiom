#include "axpch.h"

#include "UIPanel.h"

#include "Math/AxMath.h"
#include "Math/Vec.h"
#include "UI/UIContainer.h"
#include "UI/UIElement.h"
#include "UI/UISlot.h"

#include <algorithm>

namespace Axiom {
    Math::Vec2 UIPanel::getDesiredSize(const UIContext& context) {
        for (const auto& slot : slots) {
            Math::Vec2 slotSize = slot.content->getDesiredSize(context);
            if (slot.fixedSize.x() > 0) {
                slotSize.x() = slot.fixedSize.x();
            }
            if (slot.fixedSize.y() > 0) {
                slotSize.y() = slot.fixedSize.y();
            }

            slotSize.x() += slot.margin.left + slot.margin.right;
            slotSize.y() += slot.margin.top + slot.margin.bottom;

            desiredSize.x() = std::max(desiredSize.x(), slotSize.x());
            desiredSize.y() = std::max(desiredSize.y(), slotSize.y());
        }

        desiredSize += Math::Vec2(padding.left + padding.right, padding.top + padding.bottom);
        return desiredSize;
    }

    void UIPanel::arrange(const UIContext& context, const Math::Vec2& position, const Math::Vec2& size) {
        UIElement::arrange(context, position, size);

        float startX = arrangedPosition.x() + padding.left;
        float startY = arrangedPosition.y() + padding.top;
        float avaibleWidth = arrangedSize.x() - padding.left - padding.right;
        float avaibleHeight = arrangedSize.y() - padding.top - padding.bottom;

        for (const auto& slot : slots) {
            float slotX = startX + slot.margin.left;
            float slotY = startY + slot.margin.top;
            float slotWidth = avaibleWidth - slot.margin.left - slot.margin.right;
            float slotHeight = avaibleHeight - slot.margin.top - slot.margin.right;
            Math::Vec2 slotDesiredSize = slot.content->getDesiredSize(context);
            if (slot.fixedSize.x() > 0) {
                slotDesiredSize.x() = slot.fixedSize.x();
            }
            if (slot.fixedSize.y() > 0) {
                slotDesiredSize.y() = slot.fixedSize.y();
            }

            float finalWidth = slotWidth;
            float finalHeight = slotHeight;

            switch (slot.horizontalAlignment) {
            case UIAlignment::Fill:
                break;
            case UIAlignment::Start:
                finalWidth = slotDesiredSize.x();
                break;
            case UIAlignment::Center:
                finalWidth = slotDesiredSize.x();
                slotX = startX + (avaibleWidth - finalWidth) / 2.0f;
                break;
            case UIAlignment::End:
                finalWidth = slotDesiredSize.x();
                slotX = startX + avaibleWidth - finalWidth - slot.margin.right;
                break;
            default:
                break;
            }

            switch (slot.verticalAlignment) {
            case UIAlignment::Fill:
                break;
            case UIAlignment::Start:
                finalHeight = slotDesiredSize.y();
                break;
            case UIAlignment::Center:
                finalHeight = slotDesiredSize.y();
                slotY = startY + (avaibleHeight - finalHeight) / 2.0f;
                break;
            case UIAlignment::End:
                finalHeight = slotDesiredSize.y();
                slotY = startY + avaibleHeight - finalHeight - slot.margin.bottom;
                break;
            }

            slot.content->arrange(context, Math::Vec2(slotX, slotY), Math::Vec2(finalWidth, finalHeight));
        }
    }

    void UIPanel::onRender(const UIContext& context, const Math::Rect& scissorRect) {
        context.renderer->addBasicQuad(arrangedPosition, arrangedSize, overrideBackgroundColor.value_or(context.theme->panelBackgroundColor),
                                       context.theme->borderRadius, context.layer);
        UIContainer::onRender(context, scissorRect);
    }
} // namespace Axiom
