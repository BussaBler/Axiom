#pragma once
#include "GraphicsDevice.h"

namespace Axiom {
	class Pipeline {
	public:
		virtual ~Pipeline() = default;
		virtual void bind() = 0; //TODO: add a command buffer parameter
		virtual void unbind() = 0;

		static std::unique_ptr<Pipeline> create(const std::string& name, std::vector<const std::filesystem::path>& shaderStages, GraphicsDevice& graphicsDevice);

	protected:
		Pipeline(const std::string& name) : name(name) {}

	protected:
		std::string name;
	};
}
