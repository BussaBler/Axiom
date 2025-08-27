#include "RendererSystem.h"
#include <vector>

namespace Axiom {
	std::unique_ptr<RendererContext> RendererSystem::context = nullptr;
	Math::Mat4 RendererSystem::projectionMatrix = Math::Mat4::identity();
	Math::Mat4 RendererSystem::viewMatrix = Math::Mat4::identity();
	std::shared_ptr<Texture> RendererSystem::debugTexture = nullptr;

	void RendererSystem::init(Window* window) {
		context = RendererContext::create();
		context->init(window);
		projectionMatrix = Math::Mat4::perspective(Math::toRadians(45.0f), (float)window->getWidth() / (float)window->getHeight(), 0.1f, 1000.0f);
		viewMatrix = Math::Mat4::translate(Math::Vec3(0.0f, 0.0f, -30.0f));

		const uint32_t texDimension = 256;
		const uint8_t channels = 4;
		const uint32_t tileSize = 16;
		std::vector<uint8_t> data(texDimension * texDimension * channels);
		for (uint32_t y = 0; y < texDimension; ++y) {
			for (uint32_t x = 0; x < texDimension; ++x) {
				const bool isGreen = (((x / tileSize) + (y / tileSize)) % 2) == 0;
				const uint32_t idx = (y * texDimension + x) * channels;

				data[idx + 0] = 0;                 // R
				data[idx + 1] = isGreen ? 255 : 0; // G (bright)
				data[idx + 2] = 0;                 // B
				data[idx + 3] = 255;               // A
			}
		}
		debugTexture = context->createTexture(texDimension, texDimension, channels, std::move(data));
	}

	void RendererSystem::shutdown() {
		debugTexture.reset();
		if (context) {
			context->shutdown();
			context.reset();
		}
	}

	void RendererSystem::drawFrame() {
		if (context && context->beginFrame(0.0f)) {
			context->updateGlobalState(projectionMatrix, viewMatrix, Math::Vec3(0.0f, 0.0f, 30.0f), Math::Vec4(0.1f, 0.1f, 0.1f, 1.0f), 0);
			GeometryRenderData contextData(0, Math::Mat4::rotateZ(Math::AX_QUARTER_PI));
			contextData.setTexture(0, debugTexture.get());

			context->updateObjectState(contextData);
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
