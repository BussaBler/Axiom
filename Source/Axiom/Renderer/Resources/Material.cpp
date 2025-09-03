#include "Material.h"
#include "Renderer/System/TextureSystem.h"
#include "Platform/Vulkan/VulkanContext.h"

namespace Axiom {
	void Material::load(const std::filesystem::path& materialPath) {
		auto material = FileSystem::readFileStr(materialPath);

		for (const auto& line : StringUtils::split(material, '\n')) {
			auto trimmedLine = StringUtils::trim(line);
			if (trimmedLine.empty() || trimmedLine[0] == '#') {
				continue; // Skip empty lines and comments
			}
			auto delimiterPos = trimmedLine.find('=');
			if (delimiterPos == std::string::npos) {
				continue; // Skip lines without '='
			}
			auto key = StringUtils::trim(trimmedLine.substr(0, delimiterPos));
			auto value = StringUtils::trim(trimmedLine.substr(delimiterPos + 1));

			if (key == "diffuseColor") {
				diffuseColor = Math::Vec4(value);
			} 
			else if (key == "diffuseMap") {
				if (FileSystem::exists(materialPath.parent_path().parent_path() / value)) {
					std::filesystem::path texturePath = materialPath.parent_path().parent_path() / value;
					std::string texturePathStr = texturePath.string();
					diffuseMap.texture = TextureSystem::createAndLoadTexture(texturePathStr);
				} else {
					AX_LOG_WARN("Diffuse map '{}' not found for material '{}'", value, materialPath.string());
				}
			}
		}
	}

	std::shared_ptr<Material> Material::create() {
		return std::make_shared<Material>();
	}
}
