#pragma once
#include "GlobalUbo.h"
#include "MaterialUbo.h"

namespace Axiom {
	class Instance;
	class Adapter;
	class Device;
	class Context;
	class Swapchain;
	class RenderPassCache;
	class Shader;
	class Resource;
	class Texture;

	class Renderer {
	public:
		Renderer();
		~Renderer();

		void init(void* windowHandle);
		void shutdown();
		void draw();
		void resize(uint32_t width, uint32_t height);
		void bindVertexBuffer(Resource& vertexBuffer);
		void bindIndexBuffer(Resource& indexBuffer);

	private:
		void createDefaultResources();

	private:
		std::unique_ptr<Instance> instance;
		std::unique_ptr<Adapter> adapter;
		std::unique_ptr<Device> device;
		std::unique_ptr<Context> context;
		std::unique_ptr<Swapchain> swapchain;
		std::unique_ptr<RenderPassCache> renderPassCache;
		std::unique_ptr<Texture> defaultTexture;
		// temp
		std::unique_ptr<Shader> shader;
		std::unique_ptr<Resource> vertexBuffer;
		std::unique_ptr<Resource> indexBuffer;
		std::unique_ptr<Resource> uniformBuffer;
		GlobalUbo globalUbo;
		MaterialUbo materialUbo;
		Math::Mat4 rotation = Math::Mat4::rotateZ(Math::toRadians(45.0f));
	};
}

