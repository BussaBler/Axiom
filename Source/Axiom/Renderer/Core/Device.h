#pragma once
#include "Core/Window.h"
#include "CommandPool.h"
#include "Queue.h"

namespace Axiom {
	class Device {
	public:
		Device(Window* window) : window(window) {}
		virtual ~Device() = default;

		template<typename T>
		const T getHandle() const { return std::any_cast<T>(handle); }
		CommandPool& getGraphicsCommandPool() const { return *graphicsCommandPool; }
		Queue& getGraphicsQueue() const { return *graphicsQueue; }
		Queue& getPresentQueue() const { return *presentQueue; }
		Queue& getComputeQueue() const { return *computeQueue; }

		static std::unique_ptr<Device> create(Window* window);

	protected:
		std::any handle;
		std::unique_ptr<CommandPool> graphicsCommandPool;
		std::unique_ptr<Queue> graphicsQueue;
		std::unique_ptr<Queue> presentQueue;
		std::unique_ptr<Queue> computeQueue;

		Window* window;
	};
}

