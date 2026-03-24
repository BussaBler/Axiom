#pragma once
#include "axpch.h"
#include "Event/KeyCodes.h"
#include "Font.h"
#include "Math/AxMath.h"
#include "Core/Application.h"
#include "UIVertex.h"

namespace Axiom {
	class UI {
		friend class UILayer;
	public:
		static bool button(const std::string& label, Math::Vec2 pos, Math::Vec2 size);
		static void text(const std::string& text, Math::Vec2 pos, Math::Vec4 color = Math::Vec4(1.0f), uint16_t size = 11);

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

		void addBaseQuad(const Math::Vec2& pos, const Math::Vec2& size, const Math::Vec4& color);
		void addFontQuad(const Math::Vec2& pos, const Math::Vec2& size, const Math::Vec2& uv0, const Math::Vec2& uv1, const Math::Vec4& color);
		void createBaseRenderObjects();
		void createFontRenderObjects();

	private:
		static UI* instance;

		static const uint32_t maxButtonQuads = 500;
		std::unique_ptr<Pipeline> basePipeline = nullptr;
		RenderPass baseRenderPass;
		std::unique_ptr<Buffer> baseVertexBuffer = nullptr;
		std::unique_ptr<Buffer> baseIndexBuffer = nullptr;
		std::vector<UIVertex> baseVertices;

		static const uint32_t maxFontQuads = 1000;
		Font jetBrainsMonoFont{ "Assets/Fonts/OpenSans-SemiBold.ttf" };
		std::shared_ptr<Texture> fontAtlasTexture = nullptr;
		std::unique_ptr<Pipeline> fontPipeline = nullptr;
		RenderPass fontRenderPass;
		std::unique_ptr<Buffer> fontVertexBuffer = nullptr;
		std::unique_ptr<Buffer> fontIndexBuffer = nullptr;
		std::unique_ptr<ResourceLayout> fontResourceLayout = nullptr;
		std::unique_ptr<ResourceSet> fontResourceSet = nullptr;
		std::vector<UIVertex> fontVertices;

		Math::Vec2 mousePosition = Math::Vec2::zero();
		bool isMouseButtonOneDown = false;
		bool isMouseButtonTwoDown = false;
		uint32_t hotItem = 0;
		uint32_t activeItem = 0;
		bool shouldConsumeMouse = false;
	};
}

