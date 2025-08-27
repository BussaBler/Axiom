#pragma once
#include "Core/Window.h"
#include "Math/AxMath.h"
#include "Texture.h"
#include "GeometryRenderData.h"

namespace Axiom {
	class RendererContext {
	public:
		RendererContext() = default;
		virtual ~RendererContext() = default;
		virtual void init(Window* window) = 0;
		virtual void shutdown() = 0;
		virtual bool beginFrame(float deltaTime) = 0;
		virtual void updateGlobalState(Math::Mat4 projection, Math::Mat4 view, Math::Vec3 viewPos, Math::Vec4 ambientColor, int mode) = 0;
		virtual void updateObjectState(const GeometryRenderData& data) = 0;
		virtual bool endFrame() = 0;
		virtual void onResize(uint32_t width, uint32_t height) = 0;
		virtual std::shared_ptr<Texture> createTexture(uint32_t width, uint32_t height, uint8_t channels, std::vector<uint8_t> data) = 0;

		static std::unique_ptr<RendererContext> create();

	protected:
		Window* window;
	};
}
