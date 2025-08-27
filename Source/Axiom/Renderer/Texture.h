#pragma once
#include "Math/AxMath.h"

namespace Axiom {
	class Texture {
	public:
		Texture(uint32_t width, uint32_t height, uint8_t channels) : width(width), height(height), channels(channels), generation(0) {}
		virtual ~Texture() = default;

		uint32_t getId() const { return id; }
		uint32_t getWidth() const { return width; }
		uint32_t getHeight() const { return height; }
		uint32_t getGeneration() const { return generation; }

	protected:
		uint32_t id;
		uint32_t width, height;
		uint8_t channels;
		bool hasAlpha = false;
		uint32_t generation;
	};
}

