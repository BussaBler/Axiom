#pragma once
#include "Renderer/Resources/Material.h"
#include "TextureSystem.h"

namespace Axiom {
	class MaterialSystem {
	public:
		static void init(RendererContext* rendererContext);
		static void shutdown();

		static void createMaterial(std::string_view name);
		static std::shared_ptr<Material> loadMaterial(std::string_view name);
		static std::shared_ptr<Material> loadMaterial(std::string_view name, const std::filesystem::path& materialPath);
		static std::shared_ptr<Material> createAndLoadMaterial(std::string_view name) { createMaterial(name); return loadMaterial(name); }
		static std::shared_ptr<Material> getMaterial(std::string_view name);
		static std::shared_ptr<Material> getDefaultMaterial() { return defaultMaterial; };

	private:
		static void createDefaultMaterial();

	public:
		static constexpr std::string_view DEFAULT_MATERIAL = "default";

	private:
		static RendererContext* context;
		static std::unordered_map<std::string, std::shared_ptr<Material>> materials;
		static std::shared_ptr<Material> defaultMaterial;
	};
}

