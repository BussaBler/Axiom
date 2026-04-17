#pragma once
#include "ResourceLayout.h"
#include "ResourceSet.h"
#include "Shader.h"
#include "Vertex.h"
#include "axpch.h"

namespace Axiom {
    enum class VertexInputRate { Vertex, Instance };

    enum class PrimitiveTopology { TriangleList, LineList, PointList };

    enum class PolygonMode { Fill, Line, Point };

    enum class CullMode { None, Front, Back };

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
            Shader* shader = nullptr;

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

        virtual std::unique_ptr<ResourceSet> createResourceSet(ResourceLayout* resourceLayout) = 0;
    };
} // namespace Axiom
