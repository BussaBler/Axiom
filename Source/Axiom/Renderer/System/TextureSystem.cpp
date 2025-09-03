#include "axpch.h"
#include "TextureSystem.h"

namespace Axiom {
	RendererContext* TextureSystem::context = nullptr;
	std::unordered_map<std::string, std::shared_ptr<Texture>> TextureSystem::textures{};
	std::shared_ptr<Texture> TextureSystem::defaultTexture = nullptr;

	void TextureSystem::init(RendererContext* rendererContext) {
		context = rendererContext;
		createDefaultTexture();
	}

	void TextureSystem::shutdown() {
		textures.clear();
		defaultTexture.reset();
	}

	void TextureSystem::createTexture(std::string_view name) {
		if (name == DEFAULT_TEXTURE) {
			return;
		}
		if (textures.find(std::string(name)) != textures.end()) {
			AX_LOG_WARN("Texture '{}' already exists!", name);
			return;
		}
		auto texture = Texture::create(context->getDevice());
		textures[std::string(name)] = texture;
	}

	std::shared_ptr<Texture> TextureSystem::loadTexture(std::string_view name) {
		if (name == DEFAULT_TEXTURE) {
			return getDefaultTexture();
		}
		auto it = textures.find(std::string(name));
		if (it == textures.end()) {
			AX_CORE_LOG_WARN("Texture '{}' not found!", name);
			return nullptr;
		}
		std::shared_ptr<Texture> texture = it->second;
		texture->load(std::filesystem::current_path() / "Axiom" / "Assets" / "Textures" / std::string(name));
		return texture;
	}

	std::shared_ptr<Texture> TextureSystem::loadTexture(std::string_view name, const std::filesystem::path& texturePath) {
		if (name == DEFAULT_TEXTURE) {
			return getDefaultTexture();
		}
		auto it = textures.find(std::string(name));
		if (it == textures.end()) {
			AX_CORE_LOG_WARN("Texture '{}' not found!", name);
			return nullptr;
		}
		std::shared_ptr<Texture> texture = it->second;
		texture->load(texturePath);
		return texture;
	}

	std::shared_ptr<Texture> TextureSystem::getTexture(std::string_view name) {
		if (name == DEFAULT_TEXTURE) {
			return getDefaultTexture();
		}
		auto it = textures.find(std::string(name));
		if (it != textures.end()) {
			return it->second;
		}
		return nullptr;
	}

	void TextureSystem::createDefaultTexture() {
		const uint32_t texDimension = 256;
		const uint8_t channels = 4;
		const uint32_t tileSize = 16;
		std::vector<uint8_t> data(texDimension * texDimension * channels);
		for (uint32_t y = 0; y < texDimension; ++y) {
			for (uint32_t x = 0; x < texDimension; ++x) {
				const bool isGreen = (((x / tileSize) + (y / tileSize)) % 2) == 0;
				const uint32_t idx = (y * texDimension + x) * channels;

				data[idx + 0] = 0;                 // R
				data[idx + 1] = isGreen ? 255 : 0; // G (bright)
				data[idx + 2] = 0;                 // B
				data[idx + 3] = 255;               // A
			}
		}

		defaultTexture = Texture::create(context->getDevice());
		defaultTexture->createData(texDimension, texDimension, channels, data);
	}
}
