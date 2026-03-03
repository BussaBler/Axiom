#pragma once
#include "axpch.h"
#include "Buffer.h"
#include "CommandBuffer.h"
#include "Core/Window.h"
#include "Fence.h"
#include "Pipeline.h"
#include "Semaphore.h"
#include "SwapChain.h"
#include "Texture.h"

namespace Axiom {
	enum class GraphicsApi {
		Vulkan,
		DirectX12,
	};

	class Device {
	public:
		struct CreateInfo {
			GraphicsApi api = GraphicsApi::Vulkan;

			std::string appName = "Axiom Application";
			std::string engineName = "Axiom Engine";

			uint32_t appVersionMajor = 1;
			uint32_t appVersionMinor = 0;
			uint32_t appVersionPatch = 0;
			uint32_t engineVersionMajor = 1;
			uint32_t engineVersionMinor = 0;
			uint32_t engineVersionPatch = 0;

			Window* windowObjPtr = nullptr;
		};

		Device() = default;
		virtual ~Device() = default;

		// new impl
		std::unique_ptr<Device> static create(const CreateInfo& createInfo);

		virtual std::unique_ptr<SwapChain> createSwapchain(uint32_t width, uint32_t height) = 0;
		virtual std::unique_ptr<Pipeline> createPipeline(const Pipeline::CreateInfo& pipelineCreateInfo) = 0;
		virtual std::unique_ptr<CommandBuffer> createCommandBuffer() = 0;
		virtual std::unique_ptr<Semaphore> createSemaphore() = 0;
		virtual std::unique_ptr<Fence> createFence(bool isSignaled) = 0;
		virtual std::unique_ptr<Buffer> createBuffer() = 0;
		virtual std::unique_ptr<Texture> createTexture() = 0;
		virtual void submitCommandBuffers(const std::vector<CommandBuffer*> commandBuffers, const std::vector<Semaphore*> waitSemaphores, const std::vector<Semaphore*> signalSemaphores, Fence* signalFence) = 0;
		virtual void waitIdle() = 0;
	};
}

