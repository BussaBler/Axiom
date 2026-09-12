#pragma once

#include "Event/Event.h"
#include "Math/Vec.h"
#include "UI/UIElement.h"
#include "UI/UISlot.h"

#include <algorithm>
#include <memory>
#include <vector>

namespace Axiom {
    class UIContainer : public UIElement {
      public:
        UIContainer() = default;
        virtual ~UIContainer() = default;

        UISlot& addSlot(std::shared_ptr<UIElement> slotContent) {
            slotContent->setParent(this);
            slots.push_back(UISlot(slotContent));

            invalidateLayout();

            return slots.back();
        }
        void removeSlot(std::shared_ptr<UIElement> slotContent) {
            auto it = std::remove_if(slots.begin(), slots.end(), [&](const UISlot& slot) { return slot.content == slotContent; });
            if (it != slots.end()) {
                slotContent->setParent(nullptr);
                slots.erase(it, slots.end());
                invalidateLayout();
            }
        }
        void clearSlots() {
            for (auto& slot : slots) {
                slot.content->setParent(nullptr);
            }
            slots.clear();
            invalidateLayout();
        }
        UIContainer& setPadding(const UIEdgeInsets& newPadding) {
            padding = newPadding;
            return *this;
        }

        inline const std::vector<UISlot>& getSlots() const { return slots; }

        virtual void arrange(const UIContext& context, const Math::Vec2& position, const Math::Vec2& size) override {
            UIElement::arrange(context, position, size);
            for (const auto& slot : slots) {
                slot.content->arrange(context, position, size);
            }
        }

        virtual void onRender(const UIContext& context, const Math::Rect& scissorRect) override {
            for (const auto& slot : slots) {
                slot.content->onRender(context, scissorRect);
            }
        }
        virtual bool onEvent(Event& event) override {
            if (event.isHandled()) {
                return true;
            }

            for (auto it = slots.rbegin(); it != slots.rend(); it++) {
                if (it->content->onEvent(event)) {
                    return true;
                }
            }

            return UIElement::onEvent(event);
        }

      protected:
        std::vector<UISlot> slots;
        UIEdgeInsets padding;
    };
} // namespace Axiom
