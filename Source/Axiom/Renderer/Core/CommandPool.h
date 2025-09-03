#pragma once

namespace Axiom {
	class Device;

	class CommandPool {
		public:
		CommandPool() = default;
		virtual ~CommandPool() = default;

		template<typename T>
		const T getHandle() const { return std::any_cast<T>(handle); }

		static std::unique_ptr<CommandPool> create(Device& deviceRef, uint32_t queueFamilyIndex);

	protected:
		std::any handle;
	};
}

