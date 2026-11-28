#include "TextureRegistry.h"

#include "Asset/TextureAsset.h"
#include "Asset/UUID.h"
#include "Core/Assert.h"
#include "Core/Log.h"
#include "RenderHardwareInterface.h"
#include "Renderer/ResourceLayout.h"
#include "Renderer/ResourceSet.h"
#include "Renderer/Texture.h"

#include <cstdint>
#include <utility>
#include <vector>

namespace Axiom {
    void TextureRegistry::init() {
        for (uint32_t i = 1; i < 1000; i++) {
            freeSlots.push(i);
        }

        ResourceLayout::BindingCreateInfo texturesBindingCreateInfo = {
            .binding = Bindings::BindlessTextureBinding, .type = ResourceType::Texture2D, .stages = ShaderStage::Fragment, .count = 1000};
        ResourceLayout::BindingCreateInfo samplersBindingCreateInfo = {
            .binding = Bindings::BindlessTextureSamplersBinding, .type = ResourceType::Sampler, .stages = ShaderStage::Fragment, .count = 2};
        resourceLayout = rhi.createResourceLayout({texturesBindingCreateInfo, samplersBindingCreateInfo});
        resourceSet = rhi.createResourceSet(resourceLayout.get());

        Sampler* samplersData[] = {rhi.getNearestSampler(), rhi.getLinearSampler()};
        ResourceSet::Binding samplerUpdateBinding = {
            .binding = Bindings::BindlessTextureSamplersBinding,
            .type = ResourceType::Sampler,
            .stages = ShaderStage::Fragment,
            .data = samplersData,
        };
        resourceSet->update({samplerUpdateBinding});
    }

    void TextureRegistry::setFallbackTexture(Texture* texture) {
        fallbackTexture = texture;
        setSlot(FALLBACK_SLOT, texture);
    }

    uint32_t TextureRegistry::getOrRegister(UUID textureId) {
        if (!textureId.isValid()) {
            return FALLBACK_SLOT;
        }

        auto it = uuidToSlot.find(textureId);
        if (it != uuidToSlot.end()) {
            return it->second;
        }

        if (freeSlots.empty()) {
            AX_CORE_LOG_ERROR("TextureRegistry out of bindless slots!");
            return FALLBACK_SLOT;
        }

        std::shared_ptr<TextureAsset> asset = assetManager.getAsset<TextureAsset>(textureId);
        if (!asset || !asset->getTexture()) {
            return FALLBACK_SLOT;
        }

        uint32_t availableSlot = freeSlots.front();
        freeSlots.pop();

        setSlot(availableSlot, asset->getTexture());

        uuidToSlot[textureId] = availableSlot;
        slotToUuid[availableSlot] = textureId;

        return availableSlot;
    }

    void TextureRegistry::unregister(UUID textureId) {
        auto it = uuidToSlot.find(textureId);
        if (it == uuidToSlot.end()) {
            AX_CORE_LOG_WARN("Tried to unregister a non-registered texture");
            return;
        }

        uint32_t slotToFree = it->second;
        pendingReleases.push_back({slotToFree, frameCounter});

        slotToUuid.erase(slotToFree);
        uuidToSlot.erase(it);
    }

    std::vector<uint32_t> TextureRegistry::reserveSlots(uint32_t count) {
        AX_CORE_ASSERT(freeSlots.size() >= count, "Not enough avaible slots to reserve");

        std::vector<uint32_t> slots;
        slots.reserve(count);
        for (uint32_t i = 0; i < count; i++) {
            const uint32_t slot = freeSlots.front();
            freeSlots.pop();
            if (fallbackTexture) {
                setSlot(slot, fallbackTexture);
            }
            slots.push_back(slot);
        }

        return slots;
    }

    void TextureRegistry::beginFrame() {
        frameCounter++;
        auto isExpired = [this](const PendingRelease& release) { return frameCounter - release.releaseFrame > RenderHardwareInterface::MAX_FRAMES_IN_FLIGHT; };
        for (const PendingRelease& release : pendingReleases) {
            if (isExpired(release)) {
                if (fallbackTexture) {
                    setSlot(release.slot, fallbackTexture);
                }
                freeSlots.push(release.slot);
            }
        }
        std::erase_if(pendingReleases, isExpired);
    }

    void TextureRegistry::setSlot(uint32_t index, Texture* texture) {
        Texture* textureData[] = {texture};
        ResourceSet::Binding bindingUpdateInfo = {.binding = Bindings::BindlessTextureBinding,
                                                  .type = ResourceType::Texture2D,
                                                  .stages = ShaderStage::Fragment,
                                                  .data = textureData,
                                                  .dataOffset = index};
        resourceSet->update({bindingUpdateInfo});
    }
} // namespace Axiom
