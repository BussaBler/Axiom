#include "RendererSystem.h"

namespace Axiom {
	std::unique_ptr<RendererContext> RendererSystem::context = nullptr;
	Math::Mat4 RendererSystem::projectionMatrix = Math::Mat4::identity();
	Math::Mat4 RendererSystem::viewMatrix = Math::Mat4::identity();

	void RendererSystem::init(Window* window) {
		context = RendererContext::create();
		context->init(window);
		projectionMatrix = Math::Mat4::perspective(Math::toRadians(45.0f), (float)window->getWidth() / (float)window->getHeight(), 0.1f, 1000.0f);
		viewMatrix = Math::Mat4::translate(Math::Vec3(0.0f, 0.0f, -30.0f));
	}

	void RendererSystem::shutdown() {
		if (context) {
			context->shutdown();
			context.reset();
		}
	}

	void RendererSystem::drawFrame() {
		if (context && context->beginFrame()) {
			context->updateGlobalState(projectionMatrix, viewMatrix, Math::Vec3(0.0f, 0.0f, 30.0f), Math::Vec4(0.1f, 0.1f, 0.1f, 1.0f), 0);
			auto model = Math::Mat4::rotateZ(Math::AX_QUARTER_PI);
			context->updateObjectState(model);

			context->endFrame();
		}
	}

	void RendererSystem::onResize(uint32_t width, uint32_t height) {
		if (context) {
			context->onResize(width, height);
			projectionMatrix = Math::Mat4::perspective(Math::toRadians(45.0f), (float)width / (float)height, 0.1f, 1000.0f);
		}
	}
}
