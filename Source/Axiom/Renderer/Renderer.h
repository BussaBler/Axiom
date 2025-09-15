#pragma once
namespace Axiom {
	class Instance;
	class Adapter;
	class Device;
	class Context;
	class Swapchain;
	class RenderPassCache;

	class Renderer {
	public:
		Renderer();
		~Renderer();

		void init(void* windowHandle);
		void shutdown();
		void draw();
		void present();
		void resize(uint32_t width, uint32_t height);

	private:
		std::unique_ptr<Instance> instance;
		std::unique_ptr<Adapter> adapter;
		std::unique_ptr<Device> device;
		std::unique_ptr<Context> context;
		std::unique_ptr<Swapchain> swapchain;
		std::unique_ptr<RenderPassCache> renderPassCache;
	};
}

