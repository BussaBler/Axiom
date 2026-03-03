#pragma once
#include "axpch.h"

namespace Axiom {
	enum class Format {
		Undefined,
		B8G8R8A8Unorm,
		R8G8B8A8Unorm,
		D24UnormS8Uint,
		D32sFloat
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

	class Pipeline {
	public:
		struct CreateInfo {
			std::filesystem::path vertexShaderPath;
			std::filesystem::path fragmentShaderPath;

			PrimitiveTopology topology = PrimitiveTopology::TriangleList;
			PolygonMode polygonMode = PolygonMode::Fill;
			CullMode cullMode = CullMode::Back;
			bool frontFaceClockwise = true;

			bool enableBlending = false;
			bool enableDepthTest = true;
			bool enableDepthWrite = true;

			std::vector<Format> colorAttachmentFormats;
			Format depthAttachmentFormat = Format::Undefined;
		};

		Pipeline() = default;
		virtual ~Pipeline() = default;
	};
}
