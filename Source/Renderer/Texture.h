#pragma once
#include "Buffer.h"
#include "Pipeline.h"
#include "Utils/BitMaskEnum.h"
#include "axpch.h"

namespace Axiom {
    enum class TextureUsage {
        None = 0,
        TransferSrc = 1 << 0,
        TransferDst = 1 << 1,
        Sampled = 1 << 2,
        Storage = 1 << 3,
        ColorAttachment = 1 << 4,
        DepthStencilAttachment = 1 << 5,
    };

    enum class TextureAspect { None = 0, Color = 1 << 0, Depth = 1 << 1, Stencil = 1 << 2 };

    template <> struct EnableBitMaskOperators<TextureUsage> : std::true_type {};
    template <> struct EnableBitMaskOperators<TextureAspect> : std::true_type {};

    enum class TextureState { Undefined, RenderTarget, DepthStencilTarget, ShaderResource, TransferDst, TransferSrc, Present };

    class Texture {
      public:
        Texture() = default;
        virtual ~Texture() = default;

        virtual Format getFormat() const = 0;

      public:
        struct CreateInfo {
            uint32_t width = 0;
            uint32_t height = 0;
            uint32_t mipLevels = 1;
            uint32_t arrayLayers = 1;
            Format format = Format::Undefined;
            TextureUsage usage = TextureUsage::None;
            TextureAspect aspect = TextureAspect::Color;
            TextureState initialState = TextureState::Undefined;
            MemoryUsage memoryUsage = MemoryUsage::GPUOnly;
        };

        struct Barrier {
            Texture* texture;
            TextureState oldState = TextureState::Undefined;
            TextureState newState = TextureState::Undefined;
            TextureAspect aspect = TextureAspect::None;

            uint32_t baseMipLevel = 0;
            uint32_t mipLevelCount = 1;
            uint32_t baseArrayLayer = 0;
            uint32_t arrayLayerCount = 1;
        };
    };
} // namespace Axiom