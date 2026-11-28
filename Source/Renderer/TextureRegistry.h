#pragma once

#include "Asset/AssetManager.h"
#include "Asset/UUID.h"
#include "Renderer/RenderHardwareInterface.h"
#include "Renderer/ResourceLayout.h"
#include "Renderer/ResourceSet.h"
#include "Renderer/Texture.h"

#include <cstdint>
#include <memory>
#include <queue>
#include <unordered_map>
#include <vector>

namespace Axiom {
    class TextureRegistry {
      public:
        TextureRegistry(RenderHardwareInterface& rhi, AssetManager& assetManager) : rhi(rhi), assetManager(assetManager) {}
        ~TextureRegistry() = default;

        void init();

        void setFallbackTexture(Texture* texture);
        uint32_t getOrRegister(UUID textureId);
        void unregister(UUID textureId);

        std::vector<uint32_t> reserveSlots(uint32_t count);
        void setSlot(uint32_t slot, Texture* texture);

        void beginFrame();

        ResourceLayout* getResourceLayout() const { return resourceLayout.get(); }
        ResourceSet* getResourceSet() const { return resourceSet.get(); }

      private:
        RenderHardwareInterface& rhi;
        AssetManager& assetManager;

        std::unique_ptr<ResourceLayout> resourceLayout;
        std::unique_ptr<ResourceSet> resourceSet;

        static constexpr uint32_t FALLBACK_SLOT = 0;

        std::unordered_map<UUID, uint32_t> uuidToSlot;
        std::unordered_map<uint32_t, UUID> slotToUuid;
        std::queue<uint32_t> freeSlots;

        struct PendingRelease {
            uint32_t slot;
            uint64_t releaseFrame;
        };
        std::vector<PendingRelease> pendingReleases;
        uint64_t frameCounter = 0;
        Texture* fallbackTexture = nullptr;
    };
} // namespace Axiom
