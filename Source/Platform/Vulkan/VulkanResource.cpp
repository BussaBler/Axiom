#include "axpch.h"
#include "VulkanResource.h"
#include "VulkanDevice.h"
#include "VulkanView.h"

namespace Axiom {
	uint32_t VulkanResource::idCounter = 0;
	std::mutex VulkanResource::idMutex;

	VulkanResource::VulkanResource(VulkanDevice& vkDevice)
		: device(vkDevice), memory(VK_NULL_HANDLE), currentAccessFlags(0), memorySize(0), id(-1), resourceView(nullptr) {
	}

	VulkanResource::~VulkanResource() {
		if (memory != VK_NULL_HANDLE) {
			vkFreeMemory(device.getHandle(), memory, nullptr);
			memory = VK_NULL_HANDLE;
		}
	}

	uint32_t VulkanResource::generateId() {
		std::lock_guard<std::mutex> lock(idMutex);
		return idCounter++;
	}

	VkFormat VulkanResource::getVkFormat(ResourceFormat format) {
		switch (format) {
			case Axiom::Unknown:
				return VK_FORMAT_UNDEFINED;
				break;
			case Axiom::R8_U:
				return VK_FORMAT_R8_UNORM;
				break;
			case Axiom::RG8_U:
				return VK_FORMAT_R8G8_UNORM;
				break;
			case Axiom::RGBA8_U:
				return VK_FORMAT_R8G8B8A8_UNORM;
				break;
			case Axiom::RGBA8_S:
				return VK_FORMAT_R8G8B8A8_SRGB;
				break;
			case Axiom::BRGA8_U:
				return VK_FORMAT_B8G8R8A8_UNORM;
				break;
			case Axiom::BRGA8_S:
				return VK_FORMAT_B8G8R8A8_SRGB;
				break;
			case Axiom::RGBA16_F:
				return VK_FORMAT_R16G16B16A16_SFLOAT;
				break;
			case Axiom::RGBA32_F:
				return VK_FORMAT_R32G32B32A32_SFLOAT;
				break;
			case Axiom::D24S8:
				return VK_FORMAT_D24_UNORM_S8_UINT;
				break;
			case Axiom::D32_F:
				return VK_FORMAT_D32_SFLOAT;
				break;
			case Axiom::D32S8:
				return VK_FORMAT_D32_SFLOAT_S8_UINT;
				break;
			default:
				AX_CORE_LOG_ERROR("Unknown ResourceFormat in VulkanResource::getVkFormat");
				return VK_FORMAT_UNDEFINED;
				break;
		}
	}

	ResourceFormat VulkanResource::getResourceFormat(VkFormat format) {
		switch (format) {
			case VK_FORMAT_UNDEFINED:
				return ResourceFormat::Unknown;
				break;
			case VK_FORMAT_R8_UNORM:
				return ResourceFormat::R8_U;
				break;
			case VK_FORMAT_R8G8_UNORM:
				return ResourceFormat::RG8_U;
				break;
			case VK_FORMAT_R8G8B8A8_UNORM:
				return ResourceFormat::RGBA8_U;
				break;
			case VK_FORMAT_R8G8B8A8_SRGB:
				return ResourceFormat::RGBA8_S;
				break;
			case VK_FORMAT_B8G8R8A8_UNORM:
				return ResourceFormat::BRGA8_U;
				break;
			case VK_FORMAT_B8G8R8A8_SRGB:
				return ResourceFormat::BRGA8_S;
				break;
			case VK_FORMAT_R16G16B16A16_SFLOAT:
				return ResourceFormat::RGBA16_F;
				break;
			case VK_FORMAT_R32G32B32A32_SFLOAT:
				return ResourceFormat::RGBA32_F;
				break;
			case VK_FORMAT_D24_UNORM_S8_UINT:
				return ResourceFormat::D24S8;
				break;
			case VK_FORMAT_D32_SFLOAT:
				return ResourceFormat::D32_F;
				break;
			case VK_FORMAT_D32_SFLOAT_S8_UINT:
				return ResourceFormat::D32S8;
				break;
			default:
				AX_CORE_LOG_ERROR("Unknown VkFormat in VulkanResource::getResourceFormat");
				return ResourceFormat::Unknown;
				break;
		}
	}
}
