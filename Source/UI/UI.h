#pragma once
#include "ECS/Component.h"
#include "Event/KeyEvent.h"
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
        uint32_t focusedItem = 0;

        std::unordered_map<uint32_t, bool> nodeStates;
    };

    struct UIInputState {
        Math::Vec2 mousePosition = Math::Vec2::zero();
        Math::Vec2 lastMousePosition = Math::Vec2::zero();
        Math::Vec2 mouseDelta = Math::Vec2::zero();
        bool isMouseButtonOneDown = false;
        bool isMouseButtonTwoDown = false;
        bool shouldConsumeMouse = false;
        std::string currentTextInput = "";
        uint16_t backspacesThisFrame = 0;
        bool isEnterPressed = false;
        bool shouldConsumeKeyboard = false;
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
        static void render(CommandBuffer* commandBuffer, Texture* targetTexture);

        static void beginPanel(const std::string& title, Math::Vec2 pos, Math::Vec2 size);
        static void endPanel();
        static void sameLine(float spacingOffset = -1.0f);
        static bool button(const std::string& label, const Math::Vec2& size, Math::Vec4 radii = Math::Vec4(-1.0f));
        static void text(const std::string& text, const Color& color, uint16_t size = 11);
        static void rawText(const std::string& text, Math::Vec2 pos, Math::Vec4 color = Math::Vec4(1.0f), uint16_t size = 11);
        static void inputText(const std::string& label, std::string& value, uint16_t size = 11);
        static float calcTextWidth(const std::string& text, uint16_t size = 11);
        static void checkbox(const std::string& label, bool& value);
        static void dragFloat(const std::string& label, float& value, float speed = 0.1f);
        static bool treeNode(const std::string& label);
        static void treePop();
        static void image(Texture* texture, const Math::Vec2& size);
        template <typename T> static void component(void* componentData) {
            auto& fields = TypeRegistry::getFields(typeid(T));
            for (const FieldInfo& field : fields) {
                void* fieldAdress = static_cast<char*>(componentData) + field.offset;
                switch (field.type) {
                case FieldType::Float: {
                    float* fieldValue = static_cast<float*>(fieldAdress);
                    dragFloat(field.name, *fieldValue);
                    break;
                }
                case FieldType::Vec3: {
                    Math::Vec3* fieldValue = static_cast<Math::Vec3*>(fieldAdress);
                    dragFloat(field.name + " X", fieldValue->x());
                    dragFloat(field.name + " Y", fieldValue->y());
                    dragFloat(field.name + " Z", fieldValue->z());
                    break;
                }
                default:
                    break;
                }
            }
        }
        // returns the width avaible for the current panel
        static float getAvaibleWidth();
        static const UIStyle& getCurrentStyle() { return style; }

      private:
        UI() = delete;
        ~UI() = delete;

        static void setMousePosition(Math::Vec2 pos);
        static void setMouseButtonState(KeyCode button, bool pressed);

        static bool shouldConsumeMouseEvents();
        static bool shouldConsumeKeyboardEvents();

      private:
        static std::unique_ptr<UIRenderer> renderer;

        static UIContext* context;
        static UIStyle style;
        static UIInputState inputState;

        static bool showDebugOutlines;
    };
} // namespace Axiom
