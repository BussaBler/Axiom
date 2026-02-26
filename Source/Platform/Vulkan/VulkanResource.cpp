#include "axpch.h"
#include "VulkanResource.h"
#include "VulkanDevice.h"
#include "VulkanView.h"

namespace Axiom {
	uint32_t VulkanResource::idCounter = 0;
	std::mutex VulkanResource::idMutex;

	VulkanResource::VulkanResource(VulkanDevice& vkDevice)
		: device(vkDevice), memory(nullptr), currentAccessFlags(0), memorySize(0), id(generateId()), resourceView(nullptr) {
	}

	VulkanResource::~VulkanResource() {
		if (memory) {
			device.getHandle().freeMemory(memory);
			memory = nullptr;
		}
	}

	uint32_t VulkanResource::generateId() {
		std::lock_guard<std::mutex> lock(idMutex);
		return idCounter++;
	}

	Vk::Format VulkanResource::getVkFormat(ResourceFormat format) {
		switch (format) {
			case ResourceFormat::Unknown:
				return Vk::Format::eUndefined;
				break;
			case ResourceFormat::R8_U:
				return Vk::Format::eR8Unorm;
				break;
			case ResourceFormat::RG8_U:
				return Vk::Format::eR8G8Unorm;
				break;
			case ResourceFormat::RGB8_U:
				return Vk::Format::eR8G8B8Unorm;
				break;
			case ResourceFormat::RGBA8_U:
				return Vk::Format::eR8G8B8A8Unorm;
				break;
			case ResourceFormat::RGBA8_S:
				return Vk::Format::eR8G8B8Srgb;
				break;
			case ResourceFormat::BRGA8_U:
				return Vk::Format::eB8G8R8A8Unorm;
				break;
			case ResourceFormat::BRGA8_S:
				return Vk::Format::eB8G8R8A8Srgb;
				break;
			case ResourceFormat::RGBA16_F:
				return Vk::Format::eR16G16B16A16Sfloat;
				break;
			case ResourceFormat::RGBA32_F:
				return Vk::Format::eR32G32B32A32Sfloat;
				break;
			case ResourceFormat::D24S8:
				return Vk::Format::eD24UnormS8Uint;
				break;
			case ResourceFormat::D32_F:
				return Vk::Format::eD32Sfloat;
				break;
			case ResourceFormat::D32S8:
				return Vk::Format::eD32SfloatS8Uint;
				break;
			default:
				AX_CORE_LOG_ERROR("Unknown ResourceFormat in VulkanResource::getVkFormat {}", static_cast<int>(format));
				return Vk::Format::eUndefined;
				break;
		}
	}

	ResourceFormat VulkanResource::getResourceFormat(Vk::Format format) {
		switch (format) {
			case Vk::Format::eUndefined:
				return ResourceFormat::Unknown;
				break;
			case Vk::Format::eR8Unorm:
				return ResourceFormat::R8_U;
				break;
			case Vk::Format::eR8G8Unorm:
				return ResourceFormat::RG8_U;
				break;
			case Vk::Format::eR8G8B8Unorm:
				return ResourceFormat::RGB8_U;
				break;
			case Vk::Format::eR8G8B8A8Unorm:
				return ResourceFormat::RGBA8_U;
				break;
			case Vk::Format::eR8G8B8A8Srgb:
				return ResourceFormat::RGBA8_S;
				break;
			case Vk::Format::eB8G8R8A8Unorm:
				return ResourceFormat::BRGA8_U;
				break;
			case Vk::Format::eB8G8R8Srgb:
				return ResourceFormat::BRGA8_S;
				break;
			case Vk::Format::eR16G16B16A16Sfloat:
				return ResourceFormat::RGBA16_F;
				break;
			case Vk::Format::eR32G32B32A32Sfloat:
				return ResourceFormat::RGBA32_F;
				break;
			case Vk::Format::eD24UnormS8Uint:
				return ResourceFormat::D24S8;
				break;
			case Vk::Format::eD32Sfloat:
				return ResourceFormat::D32_F;
				break;
			case Vk::Format::eD32SfloatS8Uint:
				return ResourceFormat::D32S8;
				break;
			default:
				AX_CORE_LOG_ERROR("Unknown VkFormat in VulkanResource::getResourceFormat {}", static_cast<int>(format));
				return ResourceFormat::Unknown;
				break;
		}
	}

	Vk::MemoryPropertyFlags VulkanResource::getMemoryPropertyFlags(uint32_t memoryUsage) {
		Vk::MemoryPropertyFlags flags{};
		if (memoryUsage & ResourceMemoryUsage::GPU_Only) {
			flags |= Vk::MemoryPropertyFlagBits::eDeviceLocal;
		}
		if (memoryUsage & ResourceMemoryUsage::CPU_Only) {
			flags |= Vk::MemoryPropertyFlagBits::eHostVisible | Vk::MemoryPropertyFlagBits::eHostCoherent;
		}
		if (memoryUsage & ResourceMemoryUsage::CPU_To_GPU) {
			flags |= Vk::MemoryPropertyFlagBits::eHostVisible | Vk::MemoryPropertyFlagBits::eHostCoherent;
		}
		if (memoryUsage & ResourceMemoryUsage::GPU_To_CPU) {
			flags |= Vk::MemoryPropertyFlagBits::eHostVisible | Vk::MemoryPropertyFlagBits::eHostCoherent;
		}
		return flags;
	}
}
