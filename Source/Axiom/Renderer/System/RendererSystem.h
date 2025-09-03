#pragma once
#include "Core/Window.h"
#include "Renderer/Core/RendererContext.h"
#include "TextureSystem.h"
#include "MaterialSystem.h"

namespace Axiom {
	class RendererSystem {
	public:
		static void init(Window* window);
		static void shutdown();
		static void drawFrame();
		static void onResize(uint32_t width, uint32_t height);
		static void setViewMatrix(const Math::Mat4& view) { viewMatrix = view; }

	private:
		static std::unique_ptr<RendererContext> context;
		// what will be a main camera
		static Math::Mat4 projectionMatrix;
		static Math::Mat4 viewMatrix;
		// Temp
		static std::shared_ptr<Material> testMaterial;
	};
}

