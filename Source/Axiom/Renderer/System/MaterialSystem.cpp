#include "axpch.h"
#include "MaterialSystem.h"

namespace Axiom {
	RendererContext* MaterialSystem::context = nullptr;
	std::unordered_map<std::string, std::shared_ptr<Material>> MaterialSystem::materials{};
	std::shared_ptr<Material> MaterialSystem::defaultMaterial = nullptr;

	void MaterialSystem::init(RendererContext* rendererContext) {
		MaterialSystem::context = rendererContext;
		createDefaultMaterial();
	}

	void MaterialSystem::shutdown() {
		materials.clear();
		defaultMaterial.reset();
	}

	void MaterialSystem::createMaterial(std::string_view name) {
		if (name == DEFAULT_MATERIAL) {
			return;
		}
		if (materials.find(std::string(name)) != materials.end()) {
			AX_LOG_WARN("Material '{}' already exists!", name);
			return;
		}
		auto material = std::make_shared<Material>();
		materials[std::string(name)] = material;
		context->getMaterialShader().acquireResources(material);
	}

	std::shared_ptr<Material> MaterialSystem::loadMaterial(std::string_view name) {
		if (name == DEFAULT_MATERIAL) {
			return getDefaultMaterial();
		}
		auto it = materials.find(std::string(name));
		if (it == materials.end()) {
			AX_CORE_LOG_WARN("Material '{}' not found!", name);
			return nullptr;
		}
		std::shared_ptr<Material> material = it->second;
		material->load(std::filesystem::current_path() / "Axiom" / "Assets" / "Materials" / std::string(name).append(".axmat"));
		return material;
	}

	std::shared_ptr<Material> MaterialSystem::loadMaterial(std::string_view name, const std::filesystem::path& materialPath) {
		if (name == DEFAULT_MATERIAL) {
			return getDefaultMaterial();
		}
		auto it = materials.find(std::string(name));
		if (it == materials.end()) {
			AX_CORE_LOG_WARN("Material '{}' not found!", name);
			return nullptr;
		}
		std::shared_ptr<Material> material = it->second;
		material->load(materialPath);
		return material;
	}

	std::shared_ptr<Material> MaterialSystem::getMaterial(std::string_view name) {
		if (name == DEFAULT_MATERIAL) {
			return getDefaultMaterial();
		}
		auto it = materials.find(std::string(name));
		if (it == materials.end()) {
			AX_CORE_LOG_WARN("Material '{}' not found!", name);
			return nullptr;
		}
		return it->second;
	}

	void MaterialSystem::createDefaultMaterial() {
		defaultMaterial = std::make_shared<Material>();
		defaultMaterial->setDiffuseMap({ TextureSystem::getDefaultTexture(), TextureUse::DIFFUSE });
	}
}
