#include "axpch.h"

#include "UI/UICanvas.h"

#include "Event/ApplicationEvent.h"
#include "Event/Event.h"
#include "Event/KeyEvent.h"
#include "Event/MouseEvent.h"
#include "Math/Vec.h"
#include "UI/UIContainer.h"
#include "UI/UIElement.h"

#include <memory>

namespace Axiom {
    namespace {
        template <typename Fn> UIElement* bubble(UIElement* start, Fn&& fn) {
            for (UIElement* element = start; element; element = element->getParent()) {
                if (fn(*element)) {
                    return element;
                }
            }
            return nullptr;
        }
    } // namespace

    Math::Vec2 UICanvas::measure(const UIContext& context) {
        UIContainer::measure(context);
        if (activePopup) {
            activePopup->measure(context);
        }
        return desiredSize;
    }

    void UICanvas::arrange(const UIContext& context, const Math::Vec2& position, const Math::Vec2& size) {
        currentContext = context;
        UIContainer::arrange(context, position, size);
        if (activePopup) {
            activePopup->arrange(context, popupPos, activePopup->getDesiredSize());
        }
    }

    bool UICanvas::onEvent(Event& event) {
        EventDispatcher dispatcher(event);
        dispatcher.dispatch<WindowResizeEvent>([this](const WindowResizeEvent&) {
            invalidateLayout();
            return false;
        });
        dispatcher.dispatch<MouseMovedEvent>([this](const MouseMovedEvent& e) { return handleMouseMove(e); });
        dispatcher.dispatch<MouseButtonPressedEvent>([this](const MouseButtonPressedEvent& e) { return handleMouseDown(e); });
        dispatcher.dispatch<MouseButtonReleasedEvent>([this](const MouseButtonReleasedEvent& e) { return handleMouseUp(e); });
        dispatcher.dispatch<MouseScrolledEvent>(
            [this](const MouseScrolledEvent& e) { return bubble(hovered.lock().get(), [&](UIElement& element) { return element.onScroll(e); }) != nullptr; });
        dispatcher.dispatch<KeyPressedEvent>(
            [this](const KeyPressedEvent& e) { return bubble(focused.lock().get(), [&](UIElement& element) { return element.onKeyDown(e); }) != nullptr; });
        dispatcher.dispatch<KeyTypedEvent>(
            [this](const KeyTypedEvent& e) { return bubble(focused.lock().get(), [&](UIElement& element) { return element.onTextInput(e); }) != nullptr; });
        return event.isHandled();
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
        activePopup = std::move(popup);
        activePopup->setParent(this);
        popupPos = position;
        invalidateLayout();
    }

    void UICanvas::closePopup() {
        activePopup = nullptr;
        invalidateLayout();
    }

    void UICanvas::updateLayout(const UIContext& context, const Math::Vec2& position, const Math::Vec2& size) {
        if (needsUpdate || lastPos != position || lastSize != size) {
            measure(context);
            arrange(context, position, size);
            updateHover(hitTest(lastMousePos));
            lastPos = position;
            lastSize = size;
            needsUpdate = false;
        }
    }

    bool UICanvas::handleMouseMove(const MouseMovedEvent& e) {
        const Math::Vec2 p(e.getMouseX(), e.getMouseY());
        lastMousePos = p;
        updateHover(hitTest(p));

        if (std::shared_ptr<UIElement> target = captured.lock()) {
            return target->onMouseMove(e);
        }
        return bubble(hovered.lock().get(), [&](UIElement& element) { return element.onMouseMove(e); }) != nullptr;
    }

    bool UICanvas::handleMouseDown(const MouseButtonPressedEvent& e) {
        const Math::Vec2 p(e.getMouseX(), e.getMouseY());
        if (activePopup && !activePopup->containsPoint(p)) {
            closePopup();
            return true;
        }

        UIElement* target = hitTest(p);
        UIElement* handler = bubble(target, [&](UIElement& el) { return el.onMouseDown(e); });
        captured = handler ? handler->weak_from_this() : std::weak_ptr<UIElement>{};
        setFocus(findFocusable(target));
        return handler != nullptr;
    }

    bool UICanvas::handleMouseUp(const MouseButtonReleasedEvent& e) {
        if (auto target = captured.lock()) {
            captured.reset();
            return target->onMouseUp(e);
        }
        return bubble(hitTest({e.getMouseX(), e.getMouseY()}), [&](UIElement& el) { return el.onMouseUp(e); }) != nullptr;
    }

    UIElement* UICanvas::hitTest(Math::Vec2 point) {
        if (activePopup) {
            if (UIElement* hit = activePopup->hitTest(point)) {
                return hit;
            }
        }
        return UIContainer::hitTest(point);
    }

    void UICanvas::updateHover(UIElement* newHovered) {
        std::shared_ptr<UIElement> old = hovered.lock();
        if (old.get() == newHovered) {
            return;
        }
        for (UIElement* e = old.get(); e; e = e->getParent()) {
            e->onHoverChanged(false);
        }
        for (UIElement* e = newHovered; e; e = e->getParent()) {
            e->onHoverChanged(true);
        }
        hovered = newHovered ? newHovered->weak_from_this() : std::weak_ptr<UIElement>{};
    }

    void UICanvas::setFocus(UIElement* element) {
        std::shared_ptr<UIElement> old = focused.lock();
        if (old.get() == element) {
            return;
        }
        if (old) {
            old->onFocusChanged(false);
        }
        focused = element ? element->weak_from_this() : std::weak_ptr<UIElement>{};
        if (element) {
            element->onFocusChanged(true);
        }
    }

    UIElement* UICanvas::findFocusable(UIElement* start) {
        for (UIElement* e = start; e; e = e->getParent()) {
            if (e->isFocusable()) {
                return e;
            }
        }
        return nullptr;
    }
} // namespace Axiom
