#pragma once
#include "axpch.h"
#include "Renderer/Buffer.h"
#include "Renderer/Pipeline.h"
#include "Renderer/RenderPass.h"
#include "VulkanInclude.h"

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

	inline Vk::Format AxFormatToVkFormat(Format format) {
		switch (format) {
			case Format::Undefined: return Vk::Format::eUndefined;
			case Format::B8G8R8A8Unorm: return Vk::Format::eB8G8R8A8Unorm;
			case Format::R8G8B8A8Unorm: return Vk::Format::eR8G8B8A8Unorm;
			case Format::B8G8R8A8Srgb: return Vk::Format::eB8G8R8A8Srgb;
			case Format::R8G8B8A8Srgb: return Vk::Format::eR8G8B8A8Srgb;
			case Format::D24UnormS8Uint: return Vk::Format::eD24UnormS8Uint;
			case Format::D32sFloat: return Vk::Format::eD32Sfloat;
			default: return Vk::Format::eUndefined;
		}
	}

	inline Format VkFormatToAxFormat(Vk::Format format) {
		switch (format) {
			case Vk::Format::eUndefined: return Format::Undefined;
			case Vk::Format::eB8G8R8A8Unorm: return Format::B8G8R8A8Unorm;
			case Vk::Format::eR8G8B8A8Unorm: return Format::R8G8B8A8Unorm;
			case Vk::Format::eB8G8R8A8Srgb: return Format::B8G8R8A8Srgb;
			case Vk::Format::eR8G8B8A8Srgb: return Format::R8G8B8A8Srgb;
			case Vk::Format::eD24UnormS8Uint: return Format::D24UnormS8Uint;
			case Vk::Format::eD32Sfloat: return Format::D32sFloat;
			default: return Format::Undefined;
		}
	}

	inline Vk::AttachmentLoadOp AxToVkLoadOp(Axiom::LoadOp op) {
		switch (op) {
			case Axiom::LoadOp::Load: return Vk::AttachmentLoadOp::eLoad;
			case Axiom::LoadOp::Clear: return Vk::AttachmentLoadOp::eClear;
			case Axiom::LoadOp::DontCare: return Vk::AttachmentLoadOp::eDontCare;
			default: return Vk::AttachmentLoadOp::eDontCare;
		}
	}

	inline Vk::AttachmentStoreOp AxToVkStoreOp(Axiom::StoreOp op) {
		switch (op) {
			case Axiom::StoreOp::Store: return Vk::AttachmentStoreOp::eStore;
			case Axiom::StoreOp::DontCare: return Vk::AttachmentStoreOp::eDontCare;
			default: return Vk::AttachmentStoreOp::eDontCare;
		}
	}

	inline Vk::BufferUsageFlags AxToVkBufferUsage(BufferUsage usage) {
		Vk::BufferUsageFlags flags = {};
		if (usage & BufferUsage::Vertex) flags |= Vk::BufferUsageFlagBits::eVertexBuffer;
		if (usage & BufferUsage::Index) flags |= Vk::BufferUsageFlagBits::eIndexBuffer;
		if (usage & BufferUsage::Uniform) flags |= Vk::BufferUsageFlagBits::eUniformBuffer;
		if (usage & BufferUsage::Storage) flags |= Vk::BufferUsageFlagBits::eStorageBuffer;

		return flags;
	}

	inline Vk::MemoryPropertyFlags AxToVkMemProperty(MemoryUsage usage) {
		Vk::MemoryPropertyFlags flags = {};
		if (usage & MemoryUsage::GPUOnly) flags |= Vk::MemoryPropertyFlagBits::eDeviceLocal;
		if (usage & MemoryUsage::GPUandCPU) flags |= Vk::MemoryPropertyFlagBits::eHostVisible | Vk::MemoryPropertyFlagBits::eHostCoherent;
		
		return flags;
	}
}
