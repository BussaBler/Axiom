#pragma once

#include "Math/AxMath.h"
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

        virtual Math::Vec2 measure(const UIContext& context) override {
            desiredSize = Math::Vec2::zero();
            for (auto& slot : slots) {
                slot.content->measure(context);
                const Math::Vec2 outer = slot.getOuterDesiredSize();
                desiredSize = Math::Vec2(std::max(desiredSize.x(), outer.x()), std::max(desiredSize.y(), outer.y()));
            }
            desiredSize += Math::Vec2(padding.left + padding.right, padding.top + padding.bottom);
            return desiredSize;
        }
        void arrange(const UIContext& context, const Math::Vec2& position, const Math::Vec2& size) override {
            UIElement::arrange(context, position, size);
            const Math::Rect area = getContentRect();
            for (auto& slot : slots) {
                const Math::Rect rect = slot.alignInArea(area);
                slot.content->arrange(context, rect.getPos(), rect.getSize());
            }
        }

        virtual void onRender(const UIContext& context, const Math::Rect& scissorRect) override {
            for (const auto& slot : slots) {
                slot.content->onRender(context, scissorRect);
            }
        }

        UIElement* hitTest(Math::Vec2 point) override {
            if (!containsPoint(point)) {
                return nullptr;
            }
            for (auto it = slots.begin(); it != slots.end(); it++) {
                if (UIElement* hit = it->content->hitTest(point)) {
                    return hit;
                }
            }
            return this;
        }

      protected:
        Math::Rect getContentRect() const {
            return Math::Rect(arrangedPosition + Math::Vec2(padding.left, padding.top),
                              arrangedSize - Math::Vec2(padding.left + padding.right, padding.top + padding.bottom));
        }

      protected:
        std::vector<UISlot> slots;
        UIEdgeInsets padding;
    };
} // namespace Axiom
