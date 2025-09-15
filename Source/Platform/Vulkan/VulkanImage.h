#pragma once
#include "VulkanResource.h"

namespace Axiom {
	class VulkanImage : public VulkanResource {
	public:
		VulkanImage(VulkanDevice& vkDevice, VkImage vkImage, VkImageLayout vkCurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED);
		~VulkanImage() override;

		void map(void** mappedMemory, uint32_t size, uint32_t offset) override;
		ResourceView& getView(const ResourceViewCreateInfo& resourceViewCreateInfo) override;

		void init(const ResourceCreateInfo& resourceCreateInfo);

		VkImage getHandle() const { return image; }
		VkImageLayout getCurrentLayout() const { return currentLayout; }

		static VkImageUsageFlags getVkImageUsageFlags(ResourceUsage usage);

	private:
		VkImage image;
		VkImageLayout currentLayout;
		VkFormat format;
		bool shouldClean = false;
	};
}

