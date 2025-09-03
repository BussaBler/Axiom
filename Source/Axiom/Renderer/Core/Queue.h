#pragma once

namespace Axiom {
	class Device;

	class Queue {
	public:
		Queue() = default;
		virtual ~Queue() = default;

		template<typename T>
		const T getHandle() const { return std::any_cast<T>(handle); }

		static std::unique_ptr<Queue> create(Device& deviceRef, uint32_t queueIndex);

	protected:
		std::any handle;
	};
}

