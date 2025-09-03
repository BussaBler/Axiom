#pragma once
#include "Renderer/Data/Geometry/GeometryRenderData.h"
#include "Renderer/Data/Uniform/GlobalUniformObject.h"
#include "Renderer/Data/Uniform/LocalUniformObject.h"
#include "Shader.h"

namespace Axiom {
	class MaterialShader : public Shader {
	public:
		MaterialShader() = default;
		virtual ~MaterialShader() = default;
		virtual void use(CommandBuffer& commandBuffer) override = 0;
		virtual void createPipeline(RenderPass& renderPass, uint32_t width, uint32_t height) override = 0;

		virtual void updateGlobalUniformBuffer(const GlobalUniformObject& uboData) = 0;
		virtual void updateGlobalUniformBufferState(CommandBuffer& commandBuffer, uint32_t imageIndex, float deltaTime) = 0;
		virtual void updateObjectUniformBufferState(CommandBuffer& commandBuffer, uint32_t imageIndex, const GeometryRenderData& data) = 0;
		virtual void acquireResources(std::shared_ptr<Material> material) = 0;
		virtual void releaseResources(std::shared_ptr<Material> material) = 0;

		static std::unique_ptr<MaterialShader> create(Device& device);
	};
}

