#include "axpch.h"

#include "UIDropdown.h"

#include "Event/KeyCodes.h"
#include "Event/MouseEvent.h"
#include "Math/AxMath.h"
#include "UI/UIButton.h"
#include "UI/UICanvas.h"
#include "UI/UIPanel.h"
#include "UI/UIVerticalBox.h"

namespace Axiom {
    Math::Vec2 UIDropdown::measure(const UIContext& context) {
        float maxTextWidth = 0.0f;
        for (const auto& option : options) {
            float textWidth = context.renderer->calculateTextWidth(option, context.theme->fontSize);
            maxTextWidth = std::max(textWidth, maxTextWidth);
        }

        const float indicatorWidth = context.renderer->calculateTextWidth("v", context.theme->fontSize);
        desiredSize.x() = context.theme->panelPadding.left + maxTextWidth + 2.0f * context.theme->panelPadding.right + indicatorWidth;
        desiredSize.y() = context.theme->defaultRowHeight;

        return desiredSize;
    }

    void UIDropdown::onRender(const UIContext& context, const Math::Rect& scissorRect) {
        Color backgroundColor = isOpen ? context.theme->controlActiveColor : (isHovered ? context.theme->controlHoverColor : context.theme->controlNormalColor);
        context.renderer->addBasicQuad(arrangedPosition, arrangedSize, backgroundColor, context.theme->borderRadius, context.layer);

        std::string displayText = "Select...";
        if (valueGetter) {
            int selectedIndex = valueGetter();
            if (selectedIndex >= 0 && selectedIndex < options.size()) {
                displayText = options[selectedIndex];
            }
        }

        const float mainTextYOffset = context.renderer->getCenteredTextOffset(arrangedSize.y(), context.theme->fontSize);
        Math::Vec2 textPos = arrangedPosition + Math::Vec2(context.theme->panelPadding.left, mainTextYOffset);

        context.renderer->addText(displayText, textPos, context.theme->fontSize, context.theme->textPrimary, context.layer);

        const float indicatorWidth = context.renderer->calculateTextWidth("v", context.theme->fontSize);
        Math::Vec2 indicatorPos = arrangedPosition + Math::Vec2(arrangedSize.x() - context.theme->panelPadding.right - indicatorWidth, mainTextYOffset);
        context.renderer->addText("v", indicatorPos, context.theme->fontSize, context.theme->textMuted, context.layer);
    }

    bool UIDropdown::onMouseDown(const MouseButtonPressedEvent& e) {
        return e.getMouseButton() == KeyCode::LeftButton;
    }

    bool UIDropdown::onMouseUp(const MouseButtonReleasedEvent& e) {
        UICanvas* canvas = getParentCanvas();
        if (!isHovered || !canvas) {
            return true;
        }

        auto list = std::make_shared<UIPanel>();
        auto items = std::make_shared<UIVerticalBox>();
        std::weak_ptr<UIElement> weakSelf = weak_from_this();
        for (size_t i = 0; i < options.size(); i++) {
            auto item = std::make_shared<UIButton>();
            item->setText(options[i]);
            item->setOnClick([weakSelf, i, canvas]() {
                if (auto self = std::static_pointer_cast<UIDropdown>(weakSelf.lock())) {
                    if (self->valueSetter) {
                        self->valueSetter(static_cast<int>(i));
                    }
                }
                canvas->closePopup();
            });
            items->addSlot(item).setHorizontalAlignment(UIAlignment::Fill);
        }
        list->addSlot(items);

        canvas->openPopup(list, arrangedPosition + Math::Vec2(0.0f, arrangedSize.y()));
        return true;
    }
} // namespace Axiom
