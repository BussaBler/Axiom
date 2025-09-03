#pragma once
#include "Device.h"

namespace Axiom {
	class Fence {
	public:
		Fence(bool signaled) : signaled(signaled) {}	
		virtual ~Fence() = default;
		virtual void wait(uint64_t timeout) = 0;
		virtual void reset() = 0;

		template<typename T>
		const T getHandle() const { return std::any_cast<T>(handle); }
		template<typename T>
		const T* getHandlePtr() const { return std::any_cast<T>(&handle); }

		static std::unique_ptr<Fence> create(Device& deviceRef, bool signaled = false);

	protected:
		std::any handle;
		bool signaled;
	};
}

