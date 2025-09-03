#define STB_IMAGE_IMPLEMENTATION
#include "axpch.h"
#include "Texture.h"
#include "Platform/Vulkan/VulkanTexture.h"

namespace Axiom {
	std::shared_ptr<Texture> Texture::create(Device& deviceRef) {
		return std::make_shared<VulkanTexture>(static_cast<VulkanDevice&>(deviceRef));
	}

	void Texture::load(const std::filesystem::path& texturePath) {
		int width, height, channels;
		auto data = stbi_load(texturePath.string().c_str(), &width, &height, &channels, 4);

		if (data) {
			uint32_t currentGeneration = generation;
			setGeneration(UINT32_MAX);
			setWidth(width);
			setHeight(height);
			setChannels(channels);

			uint64_t dataSize = static_cast<uint64_t>(width) * static_cast<uint64_t>(height) * 4;
			bool alpha = false;
			for (uint64_t i = 0; i < dataSize; i += 4) {
				if (data[i + 3] < 255) {
					alpha = true;
					break;
				}
			}

			if (stbi_failure_reason()) {
				AX_CORE_LOG_WARN("Failed to load texture from {}: {}", texturePath.string(), stbi_failure_reason());
				stbi__err(0, 0);
			}
			else {
				AX_CORE_LOG_INFO("Loaded texture from {} ({}x{}, {} channels, alpha: {})", texturePath.string(), width, height, channels, alpha ? "yes" : "no");
			}

			destroyData();
			createData(width, height, 4, std::vector<uint8_t>(data, data + dataSize));
			generation = currentGeneration + 1;
		}
		else {
			AX_CORE_LOG_WARN("Failed to load texture from {}: {}", texturePath.string(), stbi_failure_reason());
			stbi__err(0, 0);
		}
	}
}
