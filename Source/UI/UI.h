#pragma once
#include "axpch.h"
#include "Event/KeyCodes.h"
#include "Math/AxMath.h"
#include "Renderer/Renderer.h"
#include "UIVertex.h"

namespace Axiom {
	class UI {
		friend class UILayer;
	public:
		static bool button(const std::string& label, Math::Vec2 pos, Math::Vec2 size);

	private:
		UI();
		~UI() = default;

		static void init();
		static void shutdown();

		static void beginFrame();
		static void endFrame();
		static void render(CommandBuffer* commandBuffer);
		static void setMousePosition(Math::Vec2 pos);
		static void setMouseButtonState(KeyCode button, bool pressed);

		static bool shouldConsumeMouseEvents();

	private:
		static UI* instance;
		static const uint32_t maxQuads = 500;
		std::unique_ptr<Pipeline> pipeline = nullptr;
		RenderPass renderPass;
		std::unique_ptr<Buffer> vertexBuffer = nullptr;
		std::unique_ptr<Buffer> indexBuffer = nullptr;

		std::vector<UIVertex> vertices;

		Math::Vec2 mousePosition = Math::Vec2::zero();
		bool isMouseButtonOneDown = false;
		bool isMouseButtonTwoDown = false;
		uint32_t hotItem = 0;
		uint32_t activeItem = 0;
		bool shouldConsumeMouse = false;
	};
}

