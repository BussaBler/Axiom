#pragma once
#include "GlobalUbo.h"

namespace Axiom {
	class Instance;
	class Adapter;
	class Device;
	class Context;
	class Swapchain;
	class RenderPassCache;
	class Shader;
	class Resource;

	class Renderer {
	public:
		Renderer();
		~Renderer();

		void init(void* windowHandle);
		void shutdown();
		void draw();
		void resize(uint32_t width, uint32_t height);
		void bindVertexBuffer(Resource& vertexBuffer);
		void bindIndexBuffer(Resource& indedxBuffer);

	private:
		std::unique_ptr<Instance> instance;
		std::unique_ptr<Adapter> adapter;
		std::unique_ptr<Device> device;
		std::unique_ptr<Context> context;
		std::unique_ptr<Swapchain> swapchain;
		std::unique_ptr<RenderPassCache> renderPassCache;
		// temp
		std::unique_ptr<Shader> shader;
		std::unique_ptr<Resource> vertexBuffer;
		std::unique_ptr<Resource> indexBuffer;
		std::unique_ptr<Resource> uniformBuffer;
		GlobalUbo globalUbo;
	};
}

