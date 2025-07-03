#pragma once
#include "Logger.h"

namespace Axiom {
	class Log {
	public:
		static void init();

		static std::shared_ptr<Logger> getCoreLogger() {
			return coreLogger;
		}
		static std::shared_ptr<Logger> getClientLogger() {
			return clientLogger;
		}

	private:
		static std::shared_ptr<Logger> coreLogger;
		static std::shared_ptr<Logger> clientLogger;
	};

#define AX_CORE_LOG_TRACE(...) Log::getCoreLogger()->trace(__VA_ARGS__)

#ifndef AX_DEBUG
#define AX_CORE_LOG_DEBUG(...) Log::getCoreLogger()->debug(__VA_ARGS__)
#else
#define AX_CORE_LOG_DEBUG(...)
#endif // AX_DEBUG

#define AX_CORE_LOG_INFO(...) Log::getCoreLogger()->info(__VA_ARGS__)
#define AX_CORE_LOG_WARN(...) Log::getCoreLogger()->warn(__VA_ARGS__)
#define AX_CORE_LOG_ERROR(...) Log::getCoreLogger()->error(__VA_ARGS__)

#define AX_LOG_TRACE(...) Log::getClientLogger()->trace(__VA_ARGS__)
#ifdef AX_DEBUG
#define AX_LOG_DEBUG(...) Log::getClientLogger()->debug(__VA_ARGS__)
#else
#define AX_LOG_DEBUG(...)
#endif // AX_DEBUG

#define AX_LOG_INFO(...) Log::getClientLogger()->info(__VA_ARGS__)
#define AX_LOG_WARN(...) Log::getClientLogger()->warn(__VA_ARGS__)
#define AX_LOG_ERROR(...) Log::getClientLogger()->error(__VA_ARGS__)

}

