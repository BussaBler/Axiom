#include "axpch.h"

#include "UICanvas.h"

#include "Event/Event.h"
#include "Event/MouseEvent.h"
#include "Math/Vec.h"
#include "UI/Elements/UIContainer.h"
#include "UI/Elements/UIElement.h"

namespace Axiom {
    void UICanvas::arrange(const UIContext& context, const Math::Vec2& position, const Math::Vec2& size) {
        arrangedPosition = position;
        arrangedSize = size;
        currentContext = context;

        float startX = position.x();
        float startY = position.y();
        float availableWidth = size.x();
        float availableHeight = size.y();

        for (const auto& child : children) {
            float childX = startX;
            float childY = startY;
            float childWidth = availableWidth;
            float childHeight = availableHeight;

            float finalWidth = childWidth;
            float finalHeight = childHeight;

            switch (child->getHorizontalAlignment()) {
            case UIAlignment::Fill:
                break;
            case UIAlignment::Start:
                finalWidth = child->getDesiredSize(context).x();
                break;
            case UIAlignment::Center:
                finalWidth = child->getDesiredSize(context).x();
                childX = startX + (availableWidth / 2.0f) - (finalWidth / 2.0f);
                break;
            case UIAlignment::End:
                finalWidth = child->getDesiredSize(context).x();
                childX = startX + availableWidth - finalWidth;
                break;
            default:
                break;
            }

            switch (child->getVerticalAlignment()) {
            case UIAlignment::Fill:
                break;
            case UIAlignment::Start:
                finalHeight = child->getDesiredSize(context).y();
                break;
            case UIAlignment::Center:
                finalHeight = child->getDesiredSize(context).y();
                childY = startY + (availableHeight / 2.0f) - (finalHeight / 2.0f);
                break;
            case UIAlignment::End:
                finalHeight = child->getDesiredSize(context).y();
                childY = startY + availableHeight - finalHeight;
                break;
            default:
                break;
            }

            Math::Vec2 childPosition(childX, childY);
            Math::Vec2 childAllocSize(finalWidth, finalHeight);

            child->arrange(context, childPosition, childAllocSize);
        }
    }

    bool UICanvas::onEvent(Event& event) {
        if (activePopup) {
            EventDispatcher dispatcher(event);
            dispatcher.dispatch<MouseButtonPressedEvent>([this](const MouseButtonPressedEvent& e) {
                float mx = e.getMouseX();
                float my = e.getMouseY();
                Math::Vec2 pos = activePopup->getArrangedPosition();
                Math::Vec2 size = activePopup->getArrangedSize();

                if (mx < pos.x() || mx > pos.x() + size.x() || my < pos.y() || my > pos.y() + size.y()) {
                    closePopup();
                    return true;
                }
                return false;
            });

            if (event.isHandled()) {
                return true;
            }

            if (activePopup->onEvent(event)) {
                return true;
            }
        }
        return UIContainer::onEvent(event);
    }

    void UICanvas::onRender(const UIContext& context, const Math::Rect& scissorRect) {
        UIContainer::onRender(context, scissorRect);
        if (activePopup) {
            UIContext popupContext = context;
            popupContext.layer++;
            context.renderer->pushScissorRect(scissorRect, popupContext.layer);
            activePopup->onRender(popupContext, scissorRect);
            context.renderer->popScissorRect(popupContext.layer);
        }
    }

    void UICanvas::openPopup(std::shared_ptr<UIElement> popup, Math::Vec2 position) {
        activePopup = popup;
        popupPos = position;
        activePopup->arrange(currentContext, popupPos, activePopup->getDesiredSize(currentContext));
    }
} // namespace Axiom
