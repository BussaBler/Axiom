#pragma once
#include "Math/AxMath.h"
#include "Renderer/Resources/Material.h"

namespace Axiom {
	class GeometryRenderData {
	public:
		GeometryRenderData() = default;
		GeometryRenderData(const Math::Mat4& modelMatrix) : modelMatrix(modelMatrix), material(nullptr) {}
		~GeometryRenderData() = default;

		const Math::Mat4& getModelMatrix() const { return modelMatrix; }
		std::shared_ptr<Material> getMaterial() const { return material; }

		void setModelMatrix(const Math::Mat4& matrix) { modelMatrix = matrix; }
		void setMaterial(std::shared_ptr<Material> mat) { material = mat; }

	private:
		Math::Mat4 modelMatrix;
		std::shared_ptr<Material> material;
	};
}

