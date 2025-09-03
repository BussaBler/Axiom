#pragma once
#include "Core/Window.h"
#include "Device.h"
#include "Renderer/System/TextureSystem.h"
#include "Renderer/Data/Geometry/GeometryRenderData.h"
#include "Math/AxMath.h"
#include "MaterialShader.h"

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
		
		Device& getDevice() { return *device; }
		MaterialShader& getMaterialShader() { return *materialShader; }

		static std::unique_ptr<RendererContext> create();

	protected:
		Window* window;
		std::unique_ptr<Device> device;
		std::unique_ptr<MaterialShader> materialShader;
	};
}
