#pragma once
#include "axpch.h"

namespace Axiom {
	class Device;

	class Adapter {
	public:
		virtual std::unique_ptr<Device> createDevice() = 0;
	};
}

