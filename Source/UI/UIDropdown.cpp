#include "axpch.h"

#include "UIDropdown.h"

#include "Event/KeyCodes.h"
#include "Event/MouseEvent.h"

namespace Axiom {
    Math::Vec2 UIDropdown::getDesiredSize(const UIContext& context) {
        float maxTextWidth = 0.0f;
        for (const auto& option : options) {
            float textWidth = context.renderer->calculateTextWidth(option, context.theme->fontSize);
            maxTextWidth = std::max(textWidth, maxTextWidth);
        }

        desiredSize.x() = maxTextWidth + (30.0f);
        desiredSize.y() = context.theme->fontSize * 1.8f;

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

        float textHeight = context.renderer->calculateTextHeight(context.theme->fontSize);
        float mainTextYOffset = (arrangedSize.y() - textHeight) * 0.5f;
        Math::Vec2 textPos = arrangedPosition + Math::Vec2(10.0f, mainTextYOffset);

        context.renderer->addText(displayText, textPos, context.theme->fontSize, context.theme->textPrimary, context.layer);

        Math::Vec2 indicatorPos = arrangedPosition + Math::Vec2(arrangedSize.x() - (20.0f), mainTextYOffset);
        context.renderer->addText("v", indicatorPos, context.theme->fontSize, context.theme->textPrimary, context.layer);

        if (isOpen && !options.empty()) {
            uint8_t popupLayer = context.layer + 1;

            float itemHeight = arrangedSize.y();
            float listHeight = itemHeight * options.size();
            Math::Vec2 listPos = arrangedPosition + Math::Vec2(0.0f, arrangedSize.y());
            Math::Vec2 listSize(arrangedSize.x(), listHeight);

            context.renderer->addBasicQuad(listPos, listSize, context.theme->panelBackgroundColor, 0.0f, popupLayer);

            for (size_t i = 0; i < options.size(); i++) {
                Math::Vec2 itemPos = listPos + Math::Vec2(0.0f, i * itemHeight);
                Math::Vec2 itemSize(arrangedSize.x(), itemHeight);

                if (hoveredIndex == static_cast<int>(i)) {
                    context.renderer->addBasicQuad(itemPos, itemSize, context.theme->controlHoverColor, 0.0f, popupLayer);
                }

                Math::Vec2 itemTextPos = itemPos + Math::Vec2(10.0f, mainTextYOffset);
                context.renderer->addText(options[i], itemTextPos, context.theme->fontSize, context.theme->textPrimary, popupLayer);
            }
        }
    }

    bool UIDropdown::onEvent(Event& event) {
        if (event.isHandled()) {
            return true;
        }

        EventDispatcher dispatcher(event);
        dispatcher.dispatch<MouseMovedEvent>([this](const MouseMovedEvent& e) {
            Math::Vec2 mousePos(e.getMouseX(), e.getMouseY());

            if (isOpen) {
                float itemHeight = arrangedSize.y();
                Math::Vec2 listPos = arrangedPosition + Math::Vec2(0.0f, arrangedSize.y());
                Math::Vec2 listSize(arrangedSize.x(), itemHeight * options.size());

                if (mousePos.x() >= listPos.x() && mousePos.x() <= listPos.x() + listSize.x() && mousePos.y() >= listPos.y() &&
                    mousePos.y() <= listPos.y() + listSize.y()) {
                    hoveredIndex = static_cast<int>((mousePos.y() - listPos.y()) / itemHeight);
                    return true;
                } else {
                    hoveredIndex = -1;
                }
            }
            return false;
        });
        dispatcher.dispatch<MouseButtonPressedEvent>([this](const MouseButtonPressedEvent& e) {
            if (e.getMouseButton() == KeyCode::LeftButton) {
                if (isOpen) {
                    if (hoveredIndex >= 0) {
                        activePressedIndex = hoveredIndex;
                        return true;
                    } else if (!isHovered) {
                        isOpen = false;
                        return true;
                    }
                }

                if (isHovered) {
                    isActive = true;
                    return true;
                }
            }
            return false;
        });

        dispatcher.dispatch<MouseButtonReleasedEvent>([this](const MouseButtonReleasedEvent& e) {
            if (e.getMouseButton() == KeyCode::LeftButton) {
                if (isOpen && activePressedIndex >= 0) {
                    if (hoveredIndex == activePressedIndex) {
                        if (valueSetter) {
                            valueSetter(hoveredIndex);
                        }
                        isOpen = false;
                        invalidateLayout();
                    }
                    activePressedIndex = -1;
                    return true;
                }
                if (isActive) {
                    isActive = false;
                    if (isHovered) {
                        isOpen = !isOpen;
                        if (!isOpen) {
                            hoveredIndex = -1;
                            activePressedIndex = -1;
                        }
                        return true;
                    }
                }
            }
            return false;
        });

        return UIElement::onEvent(event);
    }
} // namespace Axiom
