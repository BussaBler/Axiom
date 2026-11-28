#pragma once

#include "Event/MouseEvent.h"
#include "Math/Vec.h"
#include "UI/UIContainer.h"
#include "UI/UIElement.h"

#include <memory>
namespace Axiom {
    class UICanvas : public UIContainer {
      public:
        UICanvas() = default;
        ~UICanvas() = default;

        Math::Vec2 measure(const UIContext& context) override;
        void arrange(const UIContext& context, const Math::Vec2& position, const Math::Vec2& size) override;
        bool onEvent(Event& event);
        void onRender(const UIContext& context, const Math::Rect& scissorRect) override;
        void invalidateLayout() override { needsUpdate = true; }
        bool isCanvas() override { return true; }

        void openPopup(std::shared_ptr<UIElement> popup, const Math::Vec2& position);
        void closePopup();
        void updateLayout(const UIContext& context, const Math::Vec2& position, const Math::Vec2& size);

        void clearFocus() { setFocus(nullptr); }
        bool hasFocus() const { return !focused.expired(); }

      private:
        bool handleMouseMove(const MouseMovedEvent& event);
        bool handleMouseDown(const MouseButtonPressedEvent& event);
        bool handleMouseUp(const MouseButtonReleasedEvent& event);

        UIElement* hitTest(Math::Vec2 point) override;
        void updateHover(UIElement* newHovered);
        void setFocus(UIElement* element);
        static UIElement* findFocusable(UIElement* start);

      private:
        std::shared_ptr<UIElement> activePopup = nullptr;
        Math::Vec2 popupPos;
        UIContext currentContext;

        bool needsUpdate = true;
        Math::Vec2 lastPos = Math::Vec2(-1.0f);
        Math::Vec2 lastSize = Math::Vec2(-1.0f);
        Math::Vec2 lastMousePos = Math::Vec2(-1.0f);

        std::weak_ptr<UIElement> hovered;
        std::weak_ptr<UIElement> focused;
        std::weak_ptr<UIElement> captured;
    };
} // namespace Axiom
