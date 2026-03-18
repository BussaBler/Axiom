#pragma once
#include "axpch.h"
#include "Vertex.h"
#include "ResourceLayout.h"

namespace Axiom {
	enum class Format {
		Undefined,
		B8G8R8A8Unorm,
		R8G8B8A8Unorm,
		B8G8R8A8Srgb,
		R8G8B8A8Srgb,
		D24UnormS8Uint,
		D32sFloat,
		R32G32Sfloat,
		R32G32B32Sfloat,
		R32G32B32A32Sfloat
	};

	enum class VertexInputRate {
		Vertex,
		Instance
	};

	enum class PrimitiveTopology {
		TriangleList,
		LineList,
		PointList
	};

	enum class PolygonMode {
		Fill,
		Line,
		Point
	};

	enum class CullMode {
		None,
		Front,
		Back
	};

	struct VertexBindingDescription {
		uint32_t binding = 0;
		uint32_t stride = 0;
		VertexInputRate inputRate = VertexInputRate::Vertex;
	};

	struct VertexAttributeDescription {
		uint32_t location = 0;
		uint32_t binding = 0;
		Format format = Format::Undefined;
		uint32_t offset = 0;
	};

	class Pipeline {
	public:
		struct CreateInfo {
			std::filesystem::path vertexShaderPath;
			std::filesystem::path fragmentShaderPath;

			std::vector<VertexBindingDescription> vertexBindings;
			std::vector<VertexAttributeDescription> vertexAttributes;

			PrimitiveTopology topology = PrimitiveTopology::TriangleList;
			PolygonMode polygonMode = PolygonMode::Fill;
			CullMode cullMode = CullMode::Back;
			bool frontFaceClockwise = true;

			bool enableBlending = false;
			bool enableDepthTest = true;
			bool enableDepthWrite = true;

			std::vector<Format> colorAttachmentFormats;
			Format depthAttachmentFormat = Format::Undefined;

			std::vector<ResourceLayout*> resourceLayouts;
		};

		Pipeline() = default;
		virtual ~Pipeline() = default;
	};
}
