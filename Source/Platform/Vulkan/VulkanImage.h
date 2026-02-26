#pragma once
#include "VulkanResource.h"

namespace Axiom {
	class VulkanCommandBuffer;

	class VulkanImage : public VulkanResource {
	public:
		VulkanImage(VulkanDevice& vkDevice, Vk::Image vkImage = VK_NULL_HANDLE, Vk::ImageLayout vkCurrentLayout = Vk::ImageLayout::eUndefined);
		~VulkanImage() override;

		void loadData(void* data, uint64_t size, uint64_t offset = 0, uint32_t flags = 0) override;
		ResourceView& getView(const ResourceViewCreateInfo& resourceViewCreateInfo);

		void init(const ResourceCreateInfo& resourceCreateInfo);

		Vk::Image getHandle() const { return image; }
		Vk::ImageLayout getCurrentLayout() const { return currentLayout; }

		static Vk::ImageUsageFlags getVkImageUsageFlags(uint32_t usage);

	private:
		void transitionLayout(VulkanCommandBuffer& commandBuffer, Vk::ImageLayout newLayout);
		void copyFromBuffer(VulkanCommandBuffer& commandBuffer, Vk::Buffer buffer) const;

	private:
		Vk::Image image;
		Vk::ImageLayout currentLayout;
		Vk::Format format;
		bool shouldClean = false;
	};
}

