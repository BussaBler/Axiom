#pragma once
#include "Renderer/Buffer.h"
#include "Renderer/Pipeline.h"
#include "Renderer/RenderPass.h"
#include "Renderer/ResourceLayout.h"
#include "Renderer/Sampler.h"
#include "Renderer/Texture.h"
#include "VulkanInclude.h"
#include "axpch.h"

namespace Axiom {
    struct QueueFamilyIndices {
        std::optional<uint32_t> graphicsFamily;
        std::optional<uint32_t> presentFamily;
        bool isComplete() const {
            return graphicsFamily.has_value() && presentFamily.has_value();
        }
    };

    struct SwapChainSupportDetails {
        Vk::SurfaceCapabilitiesKHR capabilities;
        std::vector<Vk::SurfaceFormatKHR> formats;
        std::vector<Vk::PresentModeKHR> presentModes;
    };

    inline Vk::Format axToVkFormat(Format format) {
        switch (format) {
        case Format::Undefined:
            return Vk::Format::eUndefined;
        case Format::B8G8R8A8Unorm:
            return Vk::Format::eB8G8R8A8Unorm;
        case Format::R8Unorm:
            return Vk::Format::eR8Unorm;
        case Format::R8G8Unorm:
            return Vk::Format::eR8G8Unorm;
        case Format::R8G8B8Unorm:
            return Vk::Format::eR8G8B8Unorm;
        case Format::R8G8B8A8Unorm:
            return Vk::Format::eR8G8B8A8Unorm;
        case Format::B8G8R8A8Srgb:
            return Vk::Format::eB8G8R8A8Srgb;
        case Format::R8Srgb:
            return Vk::Format::eR8Srgb;
        case Format::R8G8Srgb:
            return Vk::Format::eR8G8Srgb;
        case Format::R8G8B8Srgb:
            return Vk::Format::eR8G8B8Srgb;
        case Format::R8G8B8A8Srgb:
            return Vk::Format::eR8G8B8A8Srgb;
        case Format::D24UnormS8Uint:
            return Vk::Format::eD24UnormS8Uint;
        case Format::D32sFloat:
            return Vk::Format::eD32Sfloat;
        case Format::R32G32Sfloat:
            return Vk::Format::eR32G32Sfloat;
        case Format::R32G32B32Sfloat:
            return Vk::Format::eR32G32B32Sfloat;
        case Format::R32G32B32A32Sfloat:
            return Vk::Format::eR32G32B32A32Sfloat;
        default:
            return Vk::Format::eUndefined;
        }
    }

    inline Format vkToAxFormat(Vk::Format format) {
        switch (format) {
        case Vk::Format::eUndefined:
            return Format::Undefined;
        case Vk::Format::eB8G8R8A8Unorm:
            return Format::B8G8R8A8Unorm;
        case Vk::Format::eR8G8B8A8Unorm:
            return Format::R8G8B8A8Unorm;
        case Vk::Format::eB8G8R8A8Srgb:
            return Format::B8G8R8A8Srgb;
        case Vk::Format::eR8G8B8A8Srgb:
            return Format::R8G8B8A8Srgb;
        case Vk::Format::eD24UnormS8Uint:
            return Format::D24UnormS8Uint;
        case Vk::Format::eD32Sfloat:
            return Format::D32sFloat;
        default:
            return Format::Undefined;
        }
    }

    inline Vk::AttachmentLoadOp axToVkLoadOp(Axiom::LoadOp op) {
        switch (op) {
        case Axiom::LoadOp::Load:
            return Vk::AttachmentLoadOp::eLoad;
        case Axiom::LoadOp::Clear:
            return Vk::AttachmentLoadOp::eClear;
        case Axiom::LoadOp::DontCare:
            return Vk::AttachmentLoadOp::eDontCare;
        default:
            return Vk::AttachmentLoadOp::eDontCare;
        }
    }

    inline Vk::AttachmentStoreOp axToVkStoreOp(Axiom::StoreOp op) {
        switch (op) {
        case Axiom::StoreOp::Store:
            return Vk::AttachmentStoreOp::eStore;
        case Axiom::StoreOp::DontCare:
            return Vk::AttachmentStoreOp::eDontCare;
        default:
            return Vk::AttachmentStoreOp::eDontCare;
        }
    }

    inline Vk::BufferUsageFlags axToVkBufferUsage(BufferUsage usage) {
        Vk::BufferUsageFlags flags = {};
        if ((usage & BufferUsage::Vertex) != BufferUsage::None)
            flags |= Vk::BufferUsageFlagBits::eVertexBuffer;
        if ((usage & BufferUsage::Index) != BufferUsage::None)
            flags |= Vk::BufferUsageFlagBits::eIndexBuffer;
        if ((usage & BufferUsage::Uniform) != BufferUsage::None)
            flags |= Vk::BufferUsageFlagBits::eUniformBuffer;
        if ((usage & BufferUsage::Storage) != BufferUsage::None)
            flags |= Vk::BufferUsageFlagBits::eStorageBuffer;
        if ((usage & BufferUsage::TransferSrc) != BufferUsage::None)
            flags |= Vk::BufferUsageFlagBits::eTransferSrc;
        if ((usage & BufferUsage::TransferDst) != BufferUsage::None)
            flags |= Vk::BufferUsageFlagBits::eTransferDst;

        return flags;
    }

    inline Vk::MemoryPropertyFlags axToVkMemProperty(MemoryUsage usage) {
        Vk::MemoryPropertyFlags flags = {};
        if ((usage & MemoryUsage::GPUOnly) != MemoryUsage::None)
            flags |= Vk::MemoryPropertyFlagBits::eDeviceLocal;
        if ((usage & MemoryUsage::GPUandCPU) != MemoryUsage::None)
            flags |= Vk::MemoryPropertyFlagBits::eHostVisible | Vk::MemoryPropertyFlagBits::eHostCoherent;

        return flags;
    }

    inline Vk::DescriptorType axToVkDescriptorType(ResourceType type) {
        switch (type) {
        case ResourceType::UniformBuffer:
            return Vk::DescriptorType::eUniformBuffer;
        case ResourceType::StorageBuffer:
            return Vk::DescriptorType::eStorageBuffer;
        case ResourceType::Texture:
            return Vk::DescriptorType::eSampledImage;
        case ResourceType::Sampler:
            return Vk::DescriptorType::eSampler;
        case ResourceType::CombinedTextureSampler:
            return Vk::DescriptorType::eCombinedImageSampler;
        default:
            return Vk::DescriptorType::eUniformBuffer;
        }
    }

    inline Vk::ShaderStageFlags axToVkShaderStageFlags(ShaderStage stage) {
        Vk::ShaderStageFlags flags = {};
        if ((stage & ShaderStage::Vertex) != ShaderStage::None)
            flags |= Vk::ShaderStageFlagBits::eVertex;
        if ((stage & ShaderStage::Fragment) != ShaderStage::None)
            flags |= Vk::ShaderStageFlagBits::eFragment;
        if ((stage & ShaderStage::Compute) != ShaderStage::None)
            flags |= Vk::ShaderStageFlagBits::eCompute;
        return flags;
    }

    inline Vk::ImageLayout axToVkImageLayout(TextureState state) {
        switch (state) {
        case TextureState::Undefined:
            return Vk::ImageLayout::eUndefined;
        case TextureState::RenderTarget:
            return Vk::ImageLayout::eColorAttachmentOptimal;
        case TextureState::DepthStencilTarget:
            return Vk::ImageLayout::eDepthStencilAttachmentOptimal;
        case TextureState::ShaderResource:
            return Vk::ImageLayout::eShaderReadOnlyOptimal;
        case TextureState::TransferSrc:
            return Vk::ImageLayout::eTransferSrcOptimal;
        case TextureState::TransferDst:
            return Vk::ImageLayout::eTransferDstOptimal;
        case TextureState::Present:
            return Vk::ImageLayout::ePresentSrcKHR;
        default:
            return Vk::ImageLayout::eUndefined;
        }
    }

    inline Vk::ImageUsageFlags axToVkImageUsage(TextureUsage usage) {
        Vk::ImageUsageFlags flags = {};
        if ((usage & TextureUsage::TransferSrc) != TextureUsage::None)
            flags |= Vk::ImageUsageFlagBits::eTransferSrc;
        if ((usage & TextureUsage::TransferDst) != TextureUsage::None)
            flags |= Vk::ImageUsageFlagBits::eTransferDst;
        if ((usage & TextureUsage::Sampled) != TextureUsage::None)
            flags |= Vk::ImageUsageFlagBits::eSampled;
        if ((usage & TextureUsage::Storage) != TextureUsage::None)
            flags |= Vk::ImageUsageFlagBits::eStorage;
        if ((usage & TextureUsage::ColorAttachment) != TextureUsage::None)
            flags |= Vk::ImageUsageFlagBits::eColorAttachment;
        if ((usage & TextureUsage::DepthStencilAttachment) != TextureUsage::None)
            flags |= Vk::ImageUsageFlagBits::eDepthStencilAttachment;
        return flags;
    }

    inline Vk::ImageAspectFlags axToVkImageAspectFlags(TextureAspect aspect) {
        Vk::ImageAspectFlags flags = {};
        if ((aspect & TextureAspect::Color) != TextureAspect::None)
            flags |= Vk::ImageAspectFlagBits::eColor;
        if ((aspect & TextureAspect::Depth) != TextureAspect::None)
            flags |= Vk::ImageAspectFlagBits::eDepth;
        if ((aspect & TextureAspect::Stencil) != TextureAspect::None)
            flags |= Vk::ImageAspectFlagBits::eStencil;
        return flags;
    }

    inline Vk::SamplerAddressMode axToVkAddressMode(SamplerAddressMode mode) {
        switch (mode) {
        case SamplerAddressMode::Repeat:
            return Vk::SamplerAddressMode::eRepeat;
        case SamplerAddressMode::MirroredRepeat:
            return Vk::SamplerAddressMode::eMirroredRepeat;
        case SamplerAddressMode::ClampToEdge:
            return Vk::SamplerAddressMode::eClampToEdge;
        case SamplerAddressMode::ClampToBorder:
            return Vk::SamplerAddressMode::eClampToBorder;
        default:
            return Vk::SamplerAddressMode::eRepeat;
        }
    }

    inline Vk::Filter axToVkFilterMode(SamplerFilterMode mode) {
        switch (mode) {
        case SamplerFilterMode::Linear:
            return Vk::Filter::eLinear;
        case SamplerFilterMode::Nearest:
            return Vk::Filter::eNearest;
        default:
            return Vk::Filter::eLinear;
        }
    }

    inline Vk::SamplerMipmapMode axToVkMipmapMode(SamplerFilterMode mode) {
        switch (mode) {
        case SamplerFilterMode::Linear:
            return Vk::SamplerMipmapMode::eLinear;
        case SamplerFilterMode::Nearest:
            return Vk::SamplerMipmapMode::eNearest;
        default:
            return Vk::SamplerMipmapMode::eLinear;
        }
    }
} // namespace Axiom
