#pragma once
#include "Device.h"
#include "ImageView.h"

namespace Axiom {
	class RenderPass;

	class Framebuffer {
	public:
		Framebuffer() = default;
		virtual ~Framebuffer() = default;

		template<typename T>
		const T getHandle() const { return std::any_cast<T>(handle); }

		static std::unique_ptr<Framebuffer> create(Device& device, RenderPass& renderPassRef, std::vector<ImageView>& imageViews, uint32_t width, uint32_t height);

	protected:
		std::any handle;
	};
}

