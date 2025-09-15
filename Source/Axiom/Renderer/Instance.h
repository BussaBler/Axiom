#pragma once
#include "Core/Log.h"
#include "Adapter.h"

namespace Axiom {
	enum GraphicsAPI {
		VULKAN,
		DIRECTX12,
	};

	struct InstaceInfo {
		std::string appName;
		uint32_t appVersionMajor;
		uint32_t appVersionMinor;
		uint32_t appVersionPatch;
		std::string engineName;
		uint32_t engineVersionMajor;
		uint32_t engineVersionMinor;
		uint32_t engineVersionPatch;
	};

	class Instance {
	public:
		virtual ~Instance() = default;

		static std::unique_ptr<Instance> create(GraphicsAPI api);

		virtual void init(const InstaceInfo& info) = 0;
		virtual std::unique_ptr<Adapter> getAdapter() = 0;

	protected:
		Instance(GraphicsAPI graphicsApi) : api(graphicsApi) {}

	protected:
		GraphicsAPI api;
		inline static Instance* currentInstance = nullptr;
	};
}

