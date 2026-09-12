#include "axpch.h"

#include "UICollapsableGroup.h"

#include "Event/Event.h"
#include "Event/KeyCodes.h"
#include "Event/MouseEvent.h"
#include "Math/Color.h"
#include "Math/Vec.h"
#include "UI/UIContainer.h"
#include "UI/UIElement.h"
#include "UI/UISlot.h"

#include <algorithm>
#include <string>

namespace Axiom {
    Math::Vec2 UICollapsableGroup::getDesiredSize(const UIContext& context) {
        cachedHeaderHeight = context.theme->fontSize * 1.8f;
        desiredSize = Math::Vec2(0.0f, cachedHeaderHeight);

        std::string displayTitle = (isOpen ? "v " : "> ") + title;
        float textWidth = context.renderer->calculateTextWidth(displayTitle, context.theme->fontSize);
        desiredSize.x() = textWidth + 20.0f;

        if (isOpen) {
            for (const auto& slot : slots) {
                Math::Vec2 childSize = slot.content->getDesiredSize(context);

                if (slot.fixedSize.x() > 0) {
                    childSize.x() = slot.fixedSize.x();
                }
                if (slot.fixedSize.y() > 0) {
                    childSize.y() = slot.fixedSize.y();
                }

                desiredSize.x() = std::max(desiredSize.x(), childSize.x());
                desiredSize.y() += childSize.y();
            }
        }

        desiredSize += Math::Vec2(padding.left + padding.right, padding.top + padding.bottom);
        return desiredSize;
    }

    void UICollapsableGroup::arrange(const UIContext& context, const Math::Vec2& position, const Math::Vec2& size) {
        UIElement::arrange(context, position, size);

        if (isOpen && !slots.empty()) {
            float currentY = arrangedPosition.y() + cachedHeaderHeight + padding.top;
            float startX = arrangedPosition.x() + padding.left;
            float availableWidth = arrangedSize.x() - padding.left - padding.right;

            for (auto& slot : slots) {
                currentY += slot.margin.top;

                Math::Vec2 childDesired = slot.content->getDesiredSize(context);
                if (slot.fixedSize.x() > 0) {
                    childDesired.x() = slot.fixedSize.x();
                }
                if (slot.fixedSize.y() > 0) {
                    childDesired.y() = slot.fixedSize.y();
                }

                float finalX = startX;
                float finalWidth = availableWidth;

                switch (slot.horizontalAlignment) {
                case UIAlignment::Fill:
                    finalWidth = availableWidth - slot.margin.left - slot.margin.right;
                    finalX = startX + slot.margin.left;
                    break;
                case UIAlignment::Start:
                    finalWidth = childDesired.x();
                    finalX = startX + slot.margin.left;
                    break;
                case UIAlignment::Center:
                    finalWidth = childDesired.x();
                    finalX = startX + (availableWidth / 2.0f) - (childDesired.x() / 2.0f);
                    break;
                case UIAlignment::End:
                    finalWidth = childDesired.x();
                    finalX = startX + availableWidth - childDesired.x() - slot.margin.right;
                    break;
                }
                slot.content->arrange(context, Math::Vec2(finalX, currentY), Math::Vec2(finalWidth, childDesired.y()));
                currentY += childDesired.y() + slot.margin.bottom;
            }
        }
    }

    void UICollapsableGroup::onRender(const UIContext& context, const Math::Rect& scissorRect) {
        Math::Vec2 headerSize(arrangedSize.x(), cachedHeaderHeight);
        Color headerColor = isActive ? context.theme->controlActiveColor : (isHovered ? context.theme->controlHoverColor : context.theme->panelBackgroundColor);
        context.renderer->addBasicQuad(arrangedPosition, headerSize, headerColor, context.theme->borderRadius, context.layer);

        std::string displayTitle = (isOpen ? "v " : "> ") + title;
        float textHeight = context.renderer->calculateTextHeight(context.theme->fontSize);
        float textYOffset = (cachedHeaderHeight - textHeight) * 0.5f;
        Math::Vec2 textPos = arrangedPosition + Math::Vec2(10.0f, textYOffset);
        context.renderer->addText(displayTitle, textPos, context.theme->fontSize, context.theme->textPrimary);

        if (isOpen) {
            UIContainer::onRender(context, scissorRect);
        }
    }

    bool UICollapsableGroup::onEvent(Event& event) {
        if (event.isHandled()) {
            return true;
        }

        if (isOpen) {
            for (auto it = slots.rbegin(); it != slots.rend(); it++) {
                if (it->content->onEvent(event)) {
                    return true;
                }
            }
        }

        EventDispatcher dispatcher(event);
        dispatcher.dispatch<MouseButtonPressedEvent>([this](const MouseButtonPressedEvent& e) {
            if (isHovered && e.getMouseButton() == KeyCode::LeftButton) {
                isActive = true;
                return true;
            }
            return false;
        });
        dispatcher.dispatch<MouseButtonReleasedEvent>([this](const MouseButtonReleasedEvent& e) {
            if (isActive && e.getMouseButton() == KeyCode::LeftButton) {
                isActive = false;
                if (isHovered) {
                    isOpen = !isOpen;
                    invalidateLayout();
                }
                return true;
            }
            return false;
        });

        return UIElement::onEvent(event);
    }
} // namespace Axiom
