#pragma once
#include "Math/AxMath.h"
#include "Renderer/Core/Device.h"
// Temp
#include <stb_image.h>

namespace Axiom {
	class TexureSystem;

	enum class TextureUse {
		UKNOWN = 0x00,
		DIFFUSE = 0x01,
	};

	class Texture {
		friend class TextureSystem;
	public:
		Texture() : id(0), width(1), height(1), channels(4), hasAlpha(false), generation(UINT32_MAX) {}
		virtual ~Texture() = default;

		uint32_t getId() const { return id; }
		uint32_t getWidth() const { return width; }
		uint32_t getHeight() const { return height; }
		uint8_t getChannels() const { return channels; }
		uint32_t getGeneration() const { return generation; }

		void setId(uint32_t newId) { id = newId; }
		void setWidth(uint32_t w) { width = w; }
		void setHeight(uint32_t h) { height = h; }
		void setChannels(uint8_t c) { channels = c; hasAlpha = (c == 4); }
		void setGeneration(uint32_t gen) { generation = gen; }

	protected:
		virtual void createData(uint32_t width, uint32_t height, uint8_t channels, const std::vector<uint8_t>& data) = 0;
		virtual void destroyData() = 0;
		void load(const std::filesystem::path& texturePath);

		static std::shared_ptr<Texture> create(Device& deviceRef);

	protected:
		uint32_t id;
		uint32_t width, height;
		uint8_t channels;
		bool hasAlpha = false;
		uint32_t generation;
	};

	struct TextureMap {
		std::shared_ptr<Texture> texture = nullptr;
		TextureUse use = TextureUse::UKNOWN;
	};
}