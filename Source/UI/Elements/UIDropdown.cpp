#include "axpch.h"

#include "UIDropdown.h"

#include "Event/MouseEvent.h"

namespace Axiom {
    Math::Vec2 UIDropdown::getDesiredSize(const UIContext& context) {
        for (const auto& option : options) {
            float textWidth = context.renderer->calculateTextWidth(option, resolvedTheme->fontSize, context.dpiScale);
            if (textWidth > desiredSize.x()) {
                desiredSize.x() = textWidth;
            }
            float textHeight = context.renderer->calculateTextHeight(resolvedTheme->fontSize, context.dpiScale);
            if (textHeight > desiredSize.y()) {
                desiredSize.y() = textHeight;
            }
        }

        desiredSize.x() += padding.left + padding.right + margin.left + margin.right;
        desiredSize.y() += padding.top + padding.bottom + margin.top + margin.bottom;

        return desiredSize;
    }

    void UIDropdown::onRender(const UIContext& context, const Math::Rect& scissorRect) {
        Color backgroundColor = isOpen ? resolvedTheme->controlActiveColor : (isHovered ? resolvedTheme->controlHoverColor : resolvedTheme->controlNormalColor);
        context.renderer->addBasicQuad(arrangedPosition, arrangedSize, backgroundColor, resolvedTheme->borderRadius, context.layer);

        std::string displayText = selectedIndex >= 0 ? options[selectedIndex] : "Select...";
        float textHeight = context.renderer->calculateTextHeight(resolvedTheme->fontSize, context.dpiScale);

        float mainTextYOffset = (arrangedSize.y() - textHeight) * 0.5f;
        Math::Vec2 textPos = arrangedPosition + Math::Vec2(5.0f, mainTextYOffset);

        context.renderer->addText(displayText, textPos, resolvedTheme->fontSize, context.dpiScale, resolvedTheme->textColor, context.layer);

        if (isOpen && !options.empty()) {
            uint8_t layer = context.layer + 1;
            context.renderer->pushScissorRect(scissorRect, layer);

            float itemHeight = 24.0f;
            Math::Vec2 menuPos = arrangedPosition + Math::Vec2(0.0f, arrangedSize.y());
            Math::Vec2 menuSize(arrangedSize.x(), itemHeight * options.size());

            context.renderer->addBasicQuad(menuPos, menuSize, resolvedTheme->panelBackgroundColor, resolvedTheme->borderRadius, layer);

            float itemTextYOffset = (itemHeight - textHeight) * 0.5f;

            for (size_t i = 0; i < options.size(); i++) {
                Math::Vec2 itemPos = menuPos + Math::Vec2(0.0f, i * itemHeight);
                Math::Vec2 itemSize(menuSize.x(), itemHeight);

                Color itemBgColor = (hoveredIndex == i) ? resolvedTheme->controlHoverColor : resolvedTheme->controlNormalColor;
                if (hoveredIndex == i) {
                    context.renderer->addBasicQuad(itemPos, itemSize, itemBgColor, resolvedTheme->borderRadius, layer);
                }

                context.renderer->addText(options[i], itemPos + Math::Vec2(5.0f, itemTextYOffset), resolvedTheme->fontSize, context.dpiScale,
                                          resolvedTheme->textColor, layer);
            }
            context.renderer->popScissorRect(layer);
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
                float itemHeight = 25.0f;
                Math::Vec2 menuPos = arrangedPosition + Math::Vec2(0.0f, arrangedSize.y());
                Math::Vec2 menuSize(arrangedSize.x(), itemHeight * options.size());

                if (mousePos.x() >= menuPos.x() && mousePos.x() <= menuPos.x() + menuSize.x() && mousePos.y() >= menuPos.y() &&
                    mousePos.y() <= menuPos.y() + menuSize.y()) {
                    hoveredIndex = static_cast<int>((mousePos.y() - menuPos.y()) / itemHeight);
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
                    if (hoveredIndex != -1) {
                        activeIndex = hoveredIndex;
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
                if (isActive) {
                    isActive = false;

                    if (isHovered) {
                        isOpen = !isOpen;
                    }
                    return true;
                }

                if (isOpen && activeIndex != -1) {
                    bool successfulClick = (hoveredIndex == activeIndex);
                    int selected = activeIndex;
                    activeIndex = -1;

                    if (successfulClick) {
                        selectedIndex = selected;
                        isOpen = false;
                        if (onSelectionChanged) {
                            onSelectionChanged(selectedIndex, options[selectedIndex]);
                        }
                    }
                    return true;
                }
            }
            return false;
        });

        if (event.isHandled()) {
            return true;
        }

        return UIElement::onEvent(event);
    }
} // namespace Axiom