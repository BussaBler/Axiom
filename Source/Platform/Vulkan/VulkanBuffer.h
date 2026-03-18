#pragma once
#include "Core/Assert.h"
#include "Renderer/Buffer.h"
#include "VulkanAllocator.h"
#include "VulkanCommandBuffer.h"
#include "VulkanUtils.h"

namespace Axiom {
	class VulkanBuffer : public Buffer {
	public:
		VulkanBuffer(Vk::Device logicalDevice, const CreateInfo& createInfo);
		~VulkanBuffer() override;

		uint32_t getSize() const override;
		void setData(const void* data, uint64_t size, uint64_t offset = 0) override;

		inline Vk::Buffer getHandle() const { return buffer; }

	private:
		Vk::Device device = nullptr;
		Vk::Buffer buffer = nullptr;
		uint32_t size = 0;
		Allocation allocation;
	};
}

