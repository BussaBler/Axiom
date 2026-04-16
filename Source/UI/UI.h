#pragma once
#include "Event/MouseEvent.h"
#include "Font.h"
#include "Math/AxMath.h"
#include "Math/Color.h"
#include "UIRenderer.h"
#include "UIVertex.h"
#include "axpch.h"

namespace Axiom {
    struct UIContext {
        Math::Vec2 cursorPos = Math::Vec2::zero();
        Math::Vec2 panelPos = Math::Vec2::zero();
        Math::Vec2 panelSize = Math::Vec2::zero();
        Math::Vec2 lastItemPos = Math::Vec2::zero();
        Math::Vec2 lastItemSize = Math::Vec2::zero();

        float lineHeight = 0.0f;
        float indentLevel = 0.0f;
        Color currentColor = Color::white();

        uint32_t hotItem = 0;
        uint32_t activeItem = 0;

        std::unordered_map<uint32_t, bool> nodeStates;
    };

    struct UIInputState {
        Math::Vec2 mousePosition = Math::Vec2::zero();
        Math::Vec2 lastMousePosition = Math::Vec2::zero();
        Math::Vec2 mouseDelta = Math::Vec2::zero();
        bool isMouseButtonOneDown = false;
        bool isMouseButtonTwoDown = false;
        bool shouldConsumeMouse = false;
    };

    struct UIStyle {
        float padding = 5.0f;
        float itemSpacing = 5.0f;
        float defaultWidgetHeight = 30.0f;
        Color panelColor = Color(0.2f, 0.2f, 0.2f);
        Color headerColor = Color(0.3f, 0.3f, 0.3f);
        Color buttonColor = Color(0.4f, 0.4f, 0.4f);
        Color buttonHoverColor = Color(0.5f, 0.5f, 0.5f);
        Color buttonActiveColor = Color(0.6f, 0.6f, 0.6f);
        Color textColor = Color::white();
        float borderRadius = 6.0f;
    };

    class UI {
      public:
        static void init(UIContext& newContext);
        static void shutdown();

        inline static void setContext(UIContext* newContext) { context = newContext; }

        static void onEvent(Event& event);

        static void beginFrame();
        static void endFrame();
        static void render(CommandBuffer* commandBuffer);

        static void beginPanel(const std::string& title, Math::Vec2 pos, Math::Vec2 size);
        static void endPanel();
        static void sameLine(float spacingOffset = -1.0f);
        static bool button(const std::string& label, const Math::Vec2& size, Math::Vec4 radii = Math::Vec4(-1.0f));
        static void text(const std::string& text, const Color& color, uint16_t size = 11);
        static void rawText(const std::string& text, Math::Vec2 pos, Math::Vec4 color = Math::Vec4(1.0f), uint16_t size = 11);
        static float calcTextWidth(const std::string& text, uint16_t size = 11);
        static void checkbox(const std::string& label, bool& value);
        static void dragFloat(const std::string& label, float& value, float speed = 0.1f);
        static bool treeNode(const std::string& label);
        static void treePop();
        static void image(const Texture* texture, const Math::Vec2& size);
        // returns the width avaible for the current panel
        static float getAvaibleWidth();
        static const UIStyle& getCurrentStyle() { return style; }

      private:
        UI() = delete;
        ~UI() = delete;

        static void setMousePosition(Math::Vec2 pos);
        static void setMouseButtonState(KeyCode button, bool pressed);

        static bool shouldConsumeMouseEvents();

        void createBaseRenderObjects();
        void createFontRenderObjects();

      private:
        static std::unique_ptr<UIRenderer> renderer;

        static UIContext* context;
        static UIStyle style;
        static UIInputState inputState;

        static bool showDebugOutlines;
    };
} // namespace Axiom
