#pragma once

namespace Axiom {
	struct TextureCreateInfo {
		uint32_t width = 1;
		uint32_t height = 1;
		uint32_t channels = 1;
		void* data = nullptr;

		// TODO: Add sampler settings
	};;

	class Texture {
	public:
		virtual ~Texture() = default;

	protected:

	};
}
