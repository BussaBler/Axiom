#pragma once
#include "VulkanResource.h"

namespace Axiom {
	class VulkanImage : public VulkanResource {
	public:
		VulkanImage(VulkanDevice& vkDevice, VkImage vkImage, VkImageLayout vkCurrentLayout = VK_IMAGE_LAYOUT_UNDEFINED);
		~VulkanImage() override;

		void loadData(void* data, uint64_t size, uint64_t offset = 0, uint32_t flags = 0) override;
		ResourceView& getView(const ResourceViewCreateInfo& resourceViewCreateInfo);

		void init(const ResourceCreateInfo& resourceCreateInfo);

		VkImage getHandle() const { return image; }
		VkImageLayout getCurrentLayout() const { return currentLayout; }

		static VkImageUsageFlags getVkImageUsageFlags(uint32_t usage);

	private:
		VkImage image;
		VkImageLayout currentLayout;
		VkFormat format;
		bool shouldClean = false;
	};
}

