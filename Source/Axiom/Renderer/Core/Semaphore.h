#pragma once
#include "Device.h"

namespace Axiom {
	class Semaphore {
	public:
		Semaphore() = default;
		virtual ~Semaphore() = default;

		template<typename T>
		const T getHandle() const { return std::any_cast<T>(handle); }
		template<typename T>
		const T* getHandlePtr() const { return std::any_cast<T>(&handle); }

		static std::unique_ptr<Semaphore> create(Device& deviceRef);

	protected:
		std::any handle;
	};
}

