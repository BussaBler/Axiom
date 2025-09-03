#pragma once
#include "Renderer/Resources/Texture.h"
#include "Renderer/Core/RendererContext.h"

namespace Axiom {
	class RendererContext;

	class TextureSystem {
	public:
		static void init(RendererContext* rendererContext);
		static void shutdown();

		static void createTexture(std::string_view name);
		static std::shared_ptr<Texture> loadTexture(std::string_view name);
		static std::shared_ptr<Texture> loadTexture(std::string_view name, const std::filesystem::path& texturePath);
		static std::shared_ptr<Texture> createAndLoadTexture(std::string_view name) { createTexture(name); return loadTexture(name); }
		static std::shared_ptr<Texture> getTexture(std::string_view name);
		static std::shared_ptr<Texture> getDefaultTexture() { return defaultTexture; };

	private:
		static void createDefaultTexture();

	public:
		static constexpr std::string_view DEFAULT_TEXTURE = "default";

	private:
		static RendererContext* context;
		static std::unordered_map<std::string, std::shared_ptr<Texture>> textures;
		static std::shared_ptr<Texture> defaultTexture;
	};
}

