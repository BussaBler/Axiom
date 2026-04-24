#pragma once
#include "Asset.h"
#include "Math/AxMath.h"
#include "Renderer/Renderer.h"

namespace Axiom {
    struct MeshVertex {
        Math::Vec3 position;
        Math::Vec3 normal;
        Math::Vec2 uv;
    };

    class MeshAsset : public Asset {
      public:
        MeshAsset(UUID handle, const std::string& name, std::unique_ptr<Buffer> vertexBuffer, std::unique_ptr<Buffer> indexBuffer, uint32_t indexCount)
            : Asset(handle, AssetType::Mesh, name), vertexBuffer(std::move(vertexBuffer)), indexBuffer(std::move(indexBuffer)), indexCount(indexCount) {}
        ~MeshAsset() = default;

        inline Buffer* getVertexBuffer() const { return vertexBuffer.get(); }
        inline Buffer* getIndexBuffer() const { return indexBuffer.get(); }
        inline uint32_t getIndexCount() const { return indexCount; }

      private:
        std::unique_ptr<Buffer> vertexBuffer;
        std::unique_ptr<Buffer> indexBuffer;
        uint32_t indexCount;
    };
} // namespace Axiom