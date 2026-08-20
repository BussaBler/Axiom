#pragma once
#include "Math/Vec.h"
#include "UI/Elements/UIElement.h"
#include "UIContainer.h"

#include <memory>

namespace Axiom {
    class UICanvas : public UIContainer {
      public:
        UICanvas() = default;
        ~UICanvas() = default;

        void arrange(const UIContext& context, const Math::Vec2& position, const Math::Vec2& size) override;
        bool onEvent(Event& event) override;
        void onRender(const UIContext& context, const Math::Rect& scissorRect) override;

        void openPopup(std::shared_ptr<UIElement> popup, Math::Vec2 position);
        void closePopup() { activePopup = nullptr; }

      private:
        std::shared_ptr<UIElement> activePopup = nullptr;
        Math::Vec2 popupPos;
        UIContext currentContext;
    };
} // namespace Axiom
