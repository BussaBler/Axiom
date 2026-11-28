#include "axpch.h"

#include "UICollapsableGroup.h"

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
    namespace {
        constexpr float HEADER_CONTENT_GAP = 4.0f;
    }

    Math::Vec2 UICollapsableGroup::measure(const UIContext& context) {
        cachedHeaderHeight = context.theme->headerRowHeight;
        desiredSize = Math::Vec2(0.0f, cachedHeaderHeight);

        std::string displayTitle = (isOpen ? "v " : "> ") + title;
        float textWidth = context.renderer->calculateTextWidth(displayTitle, context.theme->fontSize);
        desiredSize.x() = textWidth + 20.0f;

        if (isOpen && !slots.empty()) {
            desiredSize.y() += HEADER_CONTENT_GAP;
            for (const auto& slot : slots) {
                slot.content->measure(context);
                const Math::Vec2 outer = slot.getOuterDesiredSize();
                desiredSize.x() = std::max(desiredSize.x(), outer.x());
                desiredSize.y() += outer.y();
            }
        }

        desiredSize += Math::Vec2(padding.left + padding.right, padding.top + padding.bottom);
        return desiredSize;
    }

    void UICollapsableGroup::arrange(const UIContext& context, const Math::Vec2& position, const Math::Vec2& size) {
        UIElement::arrange(context, position, size);

        if (isOpen && !slots.empty()) {
            const Math::Rect content = getContentRect();
            float y = content.y() + cachedHeaderHeight + HEADER_CONTENT_GAP;
            for (auto& slot : slots) {
                const float stripHeight = slot.getOuterDesiredSize().y();
                const Math::Rect strip(Math::Vec2(content.x(), y), Math::Vec2(content.width(), stripHeight));

                const Math::Rect rect = slot.alignInArea(strip);
                slot.content->arrange(context, rect.getPos(), rect.getSize());
                y += stripHeight;
            }
        }
    }

    void UICollapsableGroup::onRender(const UIContext& context, const Math::Rect& scissorRect) {
        Math::Vec2 headerSize(arrangedSize.x(), cachedHeaderHeight);
        Color headerColor = isActive ? context.theme->controlActiveColor : (isHovered ? context.theme->controlHoverColor : context.theme->headerBackgroundColor);
        context.renderer->addBasicQuad(arrangedPosition, headerSize, headerColor, context.theme->borderRadius, context.layer);

        std::string displayTitle = (isOpen ? "v " : "> ") + title;
        const float textYOffset = context.renderer->getCenteredTextOffset(cachedHeaderHeight, context.theme->fontSize);
        Math::Vec2 textPos = arrangedPosition + Math::Vec2(context.theme->panelPadding.left, textYOffset);
        context.renderer->addText(displayTitle, textPos, context.theme->fontSize, context.theme->textPrimary, context.layer);

        if (isOpen) {
            UIContainer::onRender(context, scissorRect);
        }
    }

    bool UICollapsableGroup::onMouseDown(const MouseButtonPressedEvent& event) {
        const bool inHeader = event.getMouseY() <= arrangedPosition.y() + cachedHeaderHeight;
        if (event.getMouseButton() != KeyCode::LeftButton || !inHeader) {
            return false;
        }

        isActive = true;
        return true;
    }

    bool UICollapsableGroup::onMouseUp(const MouseButtonReleasedEvent& event) {
        if (!isActive) {
            return false;
        }

        isActive = false;
        const bool inHeader = event.getMouseY() <= arrangedPosition.y() + cachedHeaderHeight;
        if (isHovered && inHeader) {
            isOpen = !isOpen;
            invalidateLayout();
        }
        return true;
    }
} // namespace Axiom
