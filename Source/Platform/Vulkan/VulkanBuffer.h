#pragma once
#include "Core/Assert.h"
#include "Renderer/Buffer.h"
#include "VulkanUtils.h"

namespace Axiom {
	class VulkanBuffer : public Buffer {
	public:
		VulkanBuffer(Vk::Device logicDevice, Vk::PhysicalDevice physicalDevice , const CreateInfo& createInfo);
		~VulkanBuffer() override;

		uint32_t getSize() const override;
		void* map() override;
		void unmap() override;

		inline Vk::Buffer getHandle() const { return buffer; }

	private:
		static uint32_t findMemoryType(Vk::PhysicalDevice physicalDevice, uint32_t typeFilter, Vk::MemoryPropertyFlags properties);

	private:
		Vk::Device device = nullptr;
		Vk::Buffer buffer = nullptr;
		Vk::DeviceMemory bufferMemory = nullptr;
		uint32_t size = 0;
	};
}

