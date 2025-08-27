#pragma once
#include "Math/AxMath.h"
#include "Texture.h"

namespace Axiom {
	class GeometryRenderData {
	public:
		GeometryRenderData() = default;
		GeometryRenderData(uint32_t objectId, const Math::Mat4& modelMatrix) : objectId(objectId), modelMatrix(modelMatrix) {
			textures.fill(nullptr);
		}
		~GeometryRenderData() = default;

		uint32_t getObjectId() const { return objectId; }
		const Math::Mat4& getModelMatrix() const { return modelMatrix; }
		void setModelMatrix(const Math::Mat4& matrix) { modelMatrix = matrix; }
		void setTexture(uint32_t slot, Texture* texture) {
			if (slot < textures.size()) {
				textures[slot] = texture;
			}
		}
		Texture* getTexture(uint32_t slot) const {
			if (slot < textures.size()) {
				return textures[slot];
			}
			return nullptr;
		}

	private:
		uint32_t objectId;
		Math::Mat4 modelMatrix;
		std::array<Texture*, 16> textures;
	};
}

