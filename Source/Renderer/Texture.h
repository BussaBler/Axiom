#pragma once
#include "axpch.h"
#include "Pipeline.h"

namespace Axiom {
	enum class TextureState {
		Undefined,
		RenderTarget,
		DepthStencilTarget,
		ShaderResource,
		TransferDst,
		TransferSrc,
		Present
	};

	class Texture {
	public:
		Texture() = default;
		virtual ~Texture() = default;

		virtual Format getFormat() const = 0;

	public:
		struct Barrier {
			std::shared_ptr<Texture> texture;
			TextureState oldState;
			TextureState newState;

			uint32_t baseMipLevel = 0;
			uint32_t mipLevelCount = 1;
			uint32_t baseArrayLayer = 0;
			uint32_t arrayLayerCount = 1;
		};
	};
}