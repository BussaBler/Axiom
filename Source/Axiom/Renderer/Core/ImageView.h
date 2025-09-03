#pragma once
#include "Image.h"

namespace Axiom {
	struct ImageViewCreateInfo;

	class ImageView {
	public:
		ImageView() = default;
		virtual ~ImageView() = default;

		template<typename T>
		const T getHandle() const { return std::any_cast<T>(handle); }
		
		static std::unique_ptr<ImageView> create(Device& deviceRef, Image& imageRef, const ImageViewCreateInfo& createInfo);

	protected:
		std::any handle;
	};
}

