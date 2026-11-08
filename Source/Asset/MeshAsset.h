#pragma once
#include "Asset.h"
#include "Math/Vec.h"

#include <cstdint>

namespace Axiom {
    struct MeshVertex {
        Math::Vec3 position;
        Math::Vec3 normal;
        Math::Vec2 uv;
    };

    class MeshAsset : public Asset {
      public:
        MeshAsset(UUID handle, const std::string& name, uint32_t vertexOffset, uint32_t indexOffset, uint32_t indexCount)
            : Asset(handle, AssetType::Mesh, name), vertexOffset(vertexOffset), indexOffset(indexOffset), indexCount(indexCount) {}
        ~MeshAsset() = default;

        void updateData(uint32_t vOffset, uint32_t iOffset, uint32_t iCount) {
            vertexOffset = vOffset;
            indexOffset = iOffset;
            indexCount = iCount;
        }

        inline uint32_t getVertexOffset() const { return vertexOffset; }
        inline uint32_t getIndexOffset() const { return indexOffset; }
        inline uint32_t getIndexCount() const { return indexCount; }

      private:
        uint32_t vertexOffset;
        uint32_t indexOffset;
        uint32_t indexCount;
    };
} // namespace Axiom
