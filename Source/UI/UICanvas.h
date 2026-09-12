#pragma once

#include "Math/Vec.h"
#include "UI/UIContainer.h"

#include <memory>
namespace Axiom {
    class UICanvas : public UIContainer {
      public:
        UICanvas() = default;
        ~UICanvas() = default;

        void arrange(const UIContext& context, const Math::Vec2& position, const Math::Vec2& size) override;
        bool onEvent(Event& event) override;
        void onRender(const UIContext& context, const Math::Rect& scissorRect) override;
        void invalidateLayout() override { needsUpdate = true; }
        bool isCanvas() override { return true; }

        void openPopup(std::shared_ptr<UIElement> popup, const Math::Vec2& position);
        void closePopup() { activePopup = nullptr; }
        void updateLayout(const UIContext& context, const Math::Vec2& position, const Math::Vec2& size) {
            if (needsUpdate || lastPos != position || lastSize != size) {
                arrange(context, position, size);
                needsUpdate = false;
            }
        }

      private:
        std::shared_ptr<UIElement> activePopup = nullptr;
        Math::Vec2 popupPos;
        UIContext currentContext;

        bool needsUpdate = true;
        Math::Vec2 lastPos = Math::Vec2(-1.0f);
        Math::Vec2 lastSize = Math::Vec2(-1.0f);
    };
} // namespace Axiom
