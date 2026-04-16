#include "UI.h"
#include "Core/Application.h"
#include "Core/CommandRegistry.h"

namespace Axiom {
    std::unique_ptr<UIRenderer> UI::renderer = nullptr;
    UIContext* UI::context = nullptr;
    UIStyle UI::style = UIStyle();
    UIInputState UI::inputState = UIInputState();
    bool UI::showDebugOutlines = false;

    void UI::init(UIContext& newContext) {
        AX_CORE_LOG_INFO("Initializing UI system...");
        renderer = std::make_unique<UIRenderer>();
        context = &newContext;

        CommandRegistry::registerCommand("toggleUIDebug", [](const std::vector<std::string>& args) {
            if (!args.empty() && args[0] == "on") {
                showDebugOutlines = true;
                CommandRegistry::log("Debug outlines enabled");
            } else if (!args.empty() && args[0] == "off") {
                showDebugOutlines = false;
                CommandRegistry::log("Debug outlines disabled");
            } else {
                CommandRegistry::log("Usage: toggleUIDebug [on/off]");
            }
        });
    }

    void UI::shutdown() {
        AX_CORE_LOG_INFO("Shutting down UI system...");
        renderer.reset();
        context = nullptr;
    }

    void UI::onEvent(Event& event) {
        EventDispatcher dispatcher(event);
        dispatcher.dispatch<MouseMovedEvent>([](const MouseMovedEvent& e) {
            setMousePosition(Math::Vec2(e.getMouseX(), e.getMouseY()));
            return shouldConsumeMouseEvents();
        });
        dispatcher.dispatch<MouseButtonPressedEvent>([](const MouseButtonPressedEvent& e) {
            setMouseButtonState(e.getMouseButton(), true);
            return shouldConsumeMouseEvents();
        });
        dispatcher.dispatch<MouseButtonReleasedEvent>([](const MouseButtonReleasedEvent& e) {
            setMouseButtonState(e.getMouseButton(), false);
            return shouldConsumeMouseEvents();
        });
    }

    void UI::beginFrame() {
        context->hotItem = 0;
        inputState.mouseDelta = inputState.mousePosition - inputState.lastMousePosition;
        inputState.lastMousePosition = inputState.mousePosition;
        renderer->clearRenderData();
    }

    void UI::endFrame() {
        if (!inputState.isMouseButtonOneDown) {
            context->activeItem = 0;
        }
        inputState.shouldConsumeMouse = (context->hotItem != 0 || context->activeItem != 0);
    }

    void UI::render(CommandBuffer* commandBuffer) {
        renderer->drawUIElements(commandBuffer);
    }

    void UI::setMousePosition(Math::Vec2 pos) {
        inputState.mousePosition = pos;
    }

    void UI::setMouseButtonState(KeyCode button, bool pressed) {
        switch (button) {
        case KeyCode::LeftButton:
            inputState.isMouseButtonOneDown = pressed;
            break;
        case KeyCode::RightButton:
            inputState.isMouseButtonTwoDown = pressed;
        default:
            break;
        }
    }

    bool UI::shouldConsumeMouseEvents() {
        return inputState.shouldConsumeMouse;
    }

    void UI::beginPanel(const std::string& title, Math::Vec2 pos, Math::Vec2 size) {
        Math::Vec4 dockingRadii(0.0f, style.borderRadius, style.borderRadius, 0.0f);
        renderer->addBasicQuad(pos, size, Color(0.2f, 0.2f, 0.2f), dockingRadii);
        renderer->addBasicQuad(pos, {size.x(), 30.0f}, Color(0.3f, 0.3f, 0.3f), dockingRadii);
        if (showDebugOutlines) {
            renderer->addDebugRect(pos, size, Color::yellow());
            Math::Vec2 cursorStart = pos + Math::Vec2(style.padding, 30.0f + style.itemSpacing);
            renderer->addBasicQuad(cursorStart, Math::Vec2(size.x() - (style.padding * 2), 1.0f), Color::red());
        }

        float titleCenterY = (30.0f / 2.0f) - (8.0f * 1.5f);
        rawText(title, pos + Math::Vec2(style.padding, titleCenterY), Color::white(), 8);
        context->panelPos = pos;
        context->panelSize = size;
        context->cursorPos = pos + Math::Vec2(style.padding, 30.0f + style.itemSpacing);
    }

    void UI::endPanel() {
        context->cursorPos = Math::Vec2::zero();
        context->panelPos = Math::Vec2::zero();
        context->panelSize = Math::Vec2::zero();
    }

    void UI::sameLine(float spacingOffset) {
        float spacing = (spacingOffset >= 0.0f) ? spacingOffset : style.itemSpacing;
        context->cursorPos.y() = context->lastItemPos.y();
        context->cursorPos.x() = context->lastItemPos.x() + context->lastItemSize.x() + spacing;
    }

    bool UI::button(const std::string& label, const Math::Vec2& size, Math::Vec4 radii) {
        uint32_t id = std::hash<std::string>{}(label);
        bool isClicked = false;

        if (radii.x() < 0.0f) {
            radii = Math::Vec4(style.borderRadius);
        }

        Math::Vec2 pos = context->cursorPos;

        if (showDebugOutlines) {
            renderer->addDebugRect(pos, size, Color::cyan());

            Math::Vec2 spacingPos = pos + Math::Vec2(0.0f, size.y());
            renderer->addBasicQuad(spacingPos, Math::Vec2(size.x(), style.itemSpacing), Color::blue());
        }

        bool isHovering = inputState.mousePosition.x() >= pos.x() && inputState.mousePosition.x() <= pos.x() + size.x() &&
                          inputState.mousePosition.y() >= pos.y() && inputState.mousePosition.y() <= pos.y() + size.y();
        if (isHovering) {
            context->hotItem = id;
            if (context->activeItem == 0 && inputState.isMouseButtonOneDown) {
                context->activeItem = id;
            }
        }

        Color color = Color(0.4f, 0.4f, 0.4f, 1.0f); // Default: Dark Gray

        if (context->hotItem == id) {
            if (context->activeItem == id) {
                color = Color(0.2f, 0.2f, 0.2f, 1.0f); // Active: Darker Gray
            } else {
                color = Color(0.6f, 0.6f, 0.6f, 1.0f); // Hover: Lighter Gray
            }
        }

        if (context->activeItem == id && context->hotItem == id && !inputState.isMouseButtonOneDown) {
            isClicked = true;
        }

        renderer->addBasicQuad(pos, size, color, radii);

        float verticalCenterOffset = (size.y() / 2.0f) - (8.0f * 1.5f);
        float horizontalCenterOffset = (size.x() / 2.0f) - ((calcTextWidth(label, 8) / 2.0f));
        rawText(label, Math::Vec2(pos.x() + horizontalCenterOffset, pos.y() + verticalCenterOffset), Color::white(), 8);

        context->cursorPos.y() += size.y() + style.itemSpacing;
        context->cursorPos.x() = context->panelPos.x() + style.padding + context->indentLevel;
        context->lastItemPos = pos;
        context->lastItemSize = size;

        return isClicked;
    }

    void UI::text(const std::string& text, const Color& color, uint16_t size) {
        Font::Atlas& atlas = renderer->getFont().getAsciiAtlas();

        uint32_t dpi = Application::getWindow()->getWindowDPI();

        float pixelSize = size * (dpi / 72.0f);
        float fontScale = pixelSize / atlas.unitsPerEm;
        float lineSpacing = pixelSize * 1.2f;

        float cursorX = context->panelPos.x() + style.padding;
        float cursorY = context->cursorPos.y() + pixelSize;

        if (showDebugOutlines) {
            renderer->addBasicQuad(Math::Vec2(context->panelPos.x(), cursorY), Math::Vec2(context->panelSize.x(), 1.0f), Color::magenta());
        }

        for (char c : text) {
            if (c == '\n') {
                cursorX = context->cursorPos.x();
                cursorY += lineSpacing;
                context->cursorPos.y() += lineSpacing;
                continue;
            }

            auto glyphIt = atlas.glyphs.find(static_cast<uint32_t>(c));
            if (glyphIt == atlas.glyphs.end()) {
                continue;
            }

            const GlyphMetrics& metrics = glyphIt->second;

            float minX = cursorX + (metrics.quadMin.x() * fontScale);
            float minY = cursorY - (metrics.quadMax.y() * fontScale);
            float maxX = cursorX + (metrics.quadMax.x() * fontScale);
            float maxY = cursorY - (metrics.quadMin.y() * fontScale);

            renderer->addFontQuad(Math::Vec2(minX, minY), Math::Vec2(maxX - minX, maxY - minY), metrics.uv0, metrics.uv1, color);

            cursorX += (metrics.advance * fontScale);
        }
        context->cursorPos.y() += lineSpacing + style.itemSpacing;
    }

    void UI::rawText(const std::string& text, Math::Vec2 pos, Math::Vec4 color, uint16_t size) {
        auto& atlas = renderer->getFont().getAsciiAtlas();

        uint32_t dpi = Application::getWindow()->getWindowDPI();

        float pixelSize = size * (dpi / 72.0f);
        float fontScale = pixelSize / atlas.unitsPerEm;
        float lineSpacing = pixelSize * 1.2f;

        float cursorX = pos.x();
        float cursorY = pos.y() + pixelSize;

        if (showDebugOutlines) {
            renderer->addBasicQuad(Math::Vec2(context->panelPos.x(), cursorY), Math::Vec2(context->panelSize.x(), 1.0f), Color::magenta());
        }

        for (char c : text) {
            if (c == '\n') {
                cursorX = pos.x();
                cursorY += lineSpacing;
                continue;
            }

            auto glyphIt = atlas.glyphs.find(static_cast<uint32_t>(c));
            if (glyphIt == atlas.glyphs.end()) {
                continue;
            }

            const GlyphMetrics& metrics = glyphIt->second;

            float minX = cursorX + (metrics.quadMin.x() * fontScale);
            float minY = cursorY - (metrics.quadMax.y() * fontScale);
            float maxX = cursorX + (metrics.quadMax.x() * fontScale);
            float maxY = cursorY - (metrics.quadMin.y() * fontScale);

            renderer->addFontQuad(Math::Vec2(minX, minY), Math::Vec2(maxX - minX, maxY - minY), metrics.uv0, metrics.uv1, color);

            cursorX += (metrics.advance * fontScale);
        }
    }

    float UI::calcTextWidth(const std::string& text, uint16_t size) {
        auto& atlas = renderer->getFont().getAsciiAtlas();

        uint32_t dpi = Application::getWindow()->getWindowDPI();

        float pixelSize = size * (dpi / 72.0f);
        float fontScale = pixelSize / atlas.unitsPerEm;

        float width = 0.0f;
        for (char c : text) {
            if (c == '\n') {
                break;
            }

            auto glyphIt = atlas.glyphs.find(static_cast<uint32_t>(c));
            if (glyphIt == atlas.glyphs.end()) {
                continue;
            }

            const GlyphMetrics& metrics = glyphIt->second;
            width += (metrics.advance * fontScale);
        }
        return width;
    }

    void UI::checkbox(const std::string& label, bool& value) {
        uint32_t id = std::hash<std::string>{}(label + "_checkbox");

        float boxSize = style.defaultWidgetHeight;
        Math::Vec2 size = Math::Vec2(boxSize, boxSize);
        Math::Vec2 pos = context->cursorPos;

        bool isHovering = inputState.mousePosition.x() >= pos.x() && inputState.mousePosition.x() <= pos.x() + boxSize &&
                          inputState.mousePosition.y() >= pos.y() && inputState.mousePosition.y() <= pos.y() + boxSize;

        if (isHovering) {
            context->hotItem = id;
            if (context->activeItem == 0 && inputState.isMouseButtonOneDown) {
                context->activeItem = id;
            }
        }

        if (context->activeItem == id && context->hotItem == id && !inputState.isMouseButtonOneDown) {
            value = !value;
        }

        Color bgColor = (context->hotItem == id) ? Color(0.3f, 0.3f, 0.3f) : Color(0.2f, 0.2f, 0.2f);
        renderer->addBasicQuad(pos, size, bgColor, style.borderRadius);

        if (value) {
            float pad = 4.0f;
            Math::Vec2 innerPos = pos + Math::Vec2(pad, pad);
            Math::Vec2 innerSize = size - Math::Vec2(pad * 2.0f, pad * 2.0f);

            renderer->addBasicQuad(innerPos, innerSize, Color(0.2f, 0.6f, 1.0f, 1.0f), style.borderRadius - 2.0f);
        }

        context->lastItemPos = pos;
        context->lastItemSize = size;

        UI::sameLine();
        float textYOffset = (boxSize / 2.0f) - (8.0f * 1.5f);
        rawText(label, context->cursorPos + Math::Vec2(0, textYOffset), Color::white(), 8);

        context->cursorPos.y() += boxSize + style.itemSpacing;
        context->cursorPos.x() = context->panelPos.x() + style.padding + context->indentLevel;
    }

    void UI::dragFloat(const std::string& label, float& value, float speed) {
        uint32_t id = std::hash<std::string>{}(label + "_dragFloat");
        bool isChanged = false;

        Math::Vec2 pos = context->cursorPos;
        Math::Vec2 size(context->panelSize.x() - (style.padding * 2.0f), style.defaultWidgetHeight);

        bool isHovering = inputState.mousePosition.x() >= pos.x() && inputState.mousePosition.x() <= pos.x() + size.x() &&
                          inputState.mousePosition.y() >= pos.y() && inputState.mousePosition.y() <= pos.y() + size.y();

        if (isHovering) {
            context->hotItem = id;
            if (context->activeItem == 0 && inputState.isMouseButtonOneDown) {
                context->activeItem = id;
            }
        }

        if (context->activeItem == id && context->hotItem == id) {
            if (inputState.isMouseButtonOneDown) {
                value += inputState.mouseDelta.x() * speed;
                isChanged = true;
            }
        }

        Color bgColor = (context->hotItem == id) ? Color(0.3f, 0.3f, 0.3f) : Color(0.2f, 0.2f, 0.2f);
        renderer->addBasicQuad(pos, size, bgColor, style.borderRadius);

        std::string labelText = std::format("{}: {:.2f}", label, value);

        float verticalCenterOffset = (size.y() / 2.0f) - (8.0f * 1.5f);
        rawText(labelText, Math::Vec2(pos.x() + style.padding, pos.y() + verticalCenterOffset), Color::white(), 8);

        context->cursorPos.y() += size.y() + style.itemSpacing;
        context->cursorPos.x() = context->panelPos.x() + style.padding + context->indentLevel;
        context->lastItemPos = pos;
        context->lastItemSize = size;
    }

    bool UI::treeNode(const std::string& label) {
        uint32_t id = std::hash<std::string>{}(label + "_treeNode");
        bool isOpen = context->nodeStates[id];

        Math::Vec2 size(getAvaibleWidth(), style.defaultWidgetHeight);
        Math::Vec2 pos = context->cursorPos;

        bool isHovering = inputState.mousePosition.x() >= pos.x() && inputState.mousePosition.x() <= pos.x() + size.x() &&
                          inputState.mousePosition.y() >= pos.y() && inputState.mousePosition.y() <= pos.y() + size.y();

        if (isHovering) {
            context->hotItem = id;
            if (context->activeItem == 0 && inputState.isMouseButtonOneDown) {
                context->activeItem = id;
            }
        }

        if (context->activeItem == id && context->hotItem == id && !inputState.isMouseButtonOneDown) {
            isOpen = !isOpen;
            context->nodeStates[id] = isOpen;
        }

        if (context->nodeStates[id]) {
            renderer->addBasicQuad(pos, size, Color(0.3f, 0.3f, 0.3f), style.borderRadius);
        } else {
            renderer->addBasicQuad(pos, size, Color(0.2f, 0.2f, 0.2f), style.borderRadius);
        }

        std::string icon = isOpen ? "v " : "> ";
        std::string fullText = icon + label;

        float verticalCenterOffset = (size.y() / 2.0f) - (8.0f * 1.5f);
        rawText(fullText, pos + Math::Vec2(style.padding, verticalCenterOffset), Color::white(), 8);

        if (isOpen) {
            context->indentLevel += 15.0f; // Push all children 15 pixels to the right
        }

        context->cursorPos.y() += size.y() + style.itemSpacing;
        context->cursorPos.x() = context->panelPos.x() + style.padding + context->indentLevel;

        context->lastItemPos = pos;
        context->lastItemSize = size;

        return isOpen;
    }

    void UI::treePop() {
        context->indentLevel -= 15.0f;
        if (context->indentLevel < 0.0f) {
            context->indentLevel = 0.0f;
        }
        context->cursorPos.x() = context->panelPos.x() + style.padding + context->indentLevel;
    }

    void UI::image(const Texture* texture, const Math::Vec2& size) {
        Math::Vec2 pos = context->cursorPos;

        renderer->addImageQuad(pos, size, texture);

        context->cursorPos.y() += size.y() + style.itemSpacing;
        context->cursorPos.x() = context->panelPos.x() + style.padding + context->indentLevel;
        context->lastItemPos = pos;
        context->lastItemSize = size;
    }

    float UI::getAvaibleWidth() {
        return context->panelSize.x() - (context->cursorPos.x() - context->panelPos.x()) - style.padding;
    }
} // namespace Axiom
