#include "axpch.h"

#include "UI/UICanvas.h"

#include "Event/ApplicationEvent.h"
#include "Event/Event.h"
#include "Event/MouseEvent.h"
#include "Math/Vec.h"
#include "UI/UIContainer.h"

namespace Axiom {
    void UICanvas::arrange(const UIContext& context, const Math::Vec2& position, const Math::Vec2& size) {
        arrangedPosition = position;
        arrangedSize = size;
        currentContext = context;

        float startX = position.x() + padding.left;
        float startY = position.y() + padding.top;
        float availableWidth = size.x() - padding.left - padding.right;
        float availableHeight = size.y() - padding.top - padding.bottom;

        for (const auto& slot : slots) {
            float slotX = startX;
            float slotY = startY;
            float finalWidth = availableWidth;
            float finalHeight = availableHeight;

            Math::Vec2 slotDesiredSize = slot.content->getDesiredSize(context);

            if (slot.fixedSize.x() > 0) {
                slotDesiredSize.x() = slot.fixedSize.x();
            }
            if (slot.fixedSize.y() > 0) {
                slotDesiredSize.y() = slot.fixedSize.y();
            }

            switch (slot.horizontalAlignment) {
            case UIAlignment::Fill:
                break;
            case UIAlignment::Start:
                finalWidth = slotDesiredSize.x();
                break;
            case UIAlignment::Center:
                finalWidth = slotDesiredSize.x();
                slotX = startX + (availableWidth / 2.0f) - (finalWidth / 2.0f);
                break;
            case UIAlignment::End:
                finalWidth = slotDesiredSize.x();
                slotX = startX + availableWidth - finalWidth;
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
                slotY = startY + (availableHeight / 2.0f) - (finalHeight / 2.0f);
                break;
            case UIAlignment::End:
                finalHeight = slotDesiredSize.y();
                slotY = startY + availableHeight - finalHeight;
                break;
            }

            slot.content->arrange(context, Math::Vec2(slotX, slotY), Math::Vec2(finalWidth, finalHeight));
        }
    }

    bool UICanvas::onEvent(Event& event) {
        if (event.isHandled()) {
            return true;
        }

        EventDispatcher dispatcher(event);
        dispatcher.dispatch<WindowResizeEvent>([this](const WindowResizeEvent& e) {
            invalidateLayout();
            return false;
        });

        if (activePopup) {
            if (activePopup->onEvent(event)) {
                return true;
            }

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

    void UICanvas::openPopup(std::shared_ptr<UIElement> popup, const Math::Vec2& position) {
        activePopup = popup;
        popupPos = position;
        activePopup->arrange(currentContext, popupPos, activePopup->getDesiredSize(currentContext));
    }
} // namespace Axiom
