#pragma once
#include "Math/AxMath.h"
#include "Utils/FileSystem.h"
#include "Utils/StringUtils.h"
#include "Texture.h"

namespace Axiom {
	class MaterialSystem;

	class Material {
		friend class MaterialSystem;
	public:
		Material() : id(0), generation(UINT32_MAX), internalId(0), diffuseColor(Math::Vec4::one()) {}
		~Material() = default;

		uint32_t getId() const { return id; }
		uint32_t getGeneration() const { return generation; }
		uint32_t getInternalId() const { return internalId; }
		const Math::Vec4& getDiffuseColor() const { return diffuseColor; }
		const TextureMap& getDiffuseMap() const { return diffuseMap; }

		void setId(uint32_t newId) { id = newId; }
		void setGeneration(uint32_t gen) { generation = gen; }
		void setInternalId(uint32_t newInternalId) { internalId = newInternalId; }
		void setDiffuseColor(const Math::Vec4& color) { diffuseColor = color; }
		void setDiffuseMap(const TextureMap& map) { diffuseMap = map; }

		static std::shared_ptr<Material> create();

	protected:
		void load(const std::filesystem::path& materialPath);

	private:
		uint32_t id;
		uint32_t generation;
		uint32_t internalId;
		Math::Vec4 diffuseColor;
		TextureMap diffuseMap;
	};
}

