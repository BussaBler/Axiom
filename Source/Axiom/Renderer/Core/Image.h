#pragma once
#include "Device.h"

namespace Axiom {
	struct ImageCreateInfo;

	class Image {
	public:
		Image() = default;
		virtual ~Image() = default;

		template <typename T>
		const T getHandle() const { return std::any_cast<T>(handle); }

		static std::unique_ptr<Image> create(Device& device, const ImageCreateInfo& createInfo);

	protected:
		std::any handle;
	};
}

