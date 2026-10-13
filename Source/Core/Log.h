#pragma once
#include "Logger.h"

namespace Axiom {
    class Log {
      public:
        static void init();

        static std::shared_ptr<Logger> getCoreLogger() { return coreLogger; }
        static std::shared_ptr<Logger> getClientLogger() { return clientLogger; }

      private:
        static std::shared_ptr<Logger> coreLogger;
        static std::shared_ptr<Logger> clientLogger;
    };

#define AX_CORE_LOG_TRACE(...)                                                                                                                                 \
    do {                                                                                                                                                       \
        if (Axiom::Log::getCoreLogger()->isEnabled(Axiom::Logger::Priority::Trace)) {                                                                          \
            Axiom::Log::getCoreLogger()->trace(__VA_ARGS__);                                                                                                   \
        }                                                                                                                                                      \
    } while (0)

#ifdef AX_DEBUG
#define AX_CORE_LOG_DEBUG(...)                                                                                                                                 \
    do {                                                                                                                                                       \
        if (Axiom::Log::getCoreLogger()->isEnabled(Axiom::Logger::Priority::Debug)) {                                                                          \
            Axiom::Log::getCoreLogger()->debug(__VA_ARGS__);                                                                                                   \
        }                                                                                                                                                      \
    } while (0)
#else
#define AX_CORE_LOG_DEBUG(...)
#endif // AX_DEBUG

#define AX_CORE_LOG_INFO(...)                                                                                                                                  \
    do {                                                                                                                                                       \
        if (Axiom::Log::getCoreLogger()->isEnabled(Axiom::Logger::Priority::Info)) {                                                                           \
            Axiom::Log::getCoreLogger()->info(__VA_ARGS__);                                                                                                    \
        }                                                                                                                                                      \
    } while (0)
#define AX_CORE_LOG_WARN(...)                                                                                                                                  \
    do {                                                                                                                                                       \
        if (Axiom::Log::getCoreLogger()->isEnabled(Axiom::Logger::Priority::Warn)) {                                                                           \
            Axiom::Log::getCoreLogger()->warn(__VA_ARGS__);                                                                                                    \
        }                                                                                                                                                      \
    } while (0)
#define AX_CORE_LOG_ERROR_ONCE(...)                                                                                                                            \
    do {                                                                                                                                                       \
        static bool wasLogged = false;                                                                                                                         \
        if (!wasLogged && Axiom::Log::getCoreLogger()->isEnabled(Axiom::Logger::Priority::Error)) {                                                            \
            Axiom::Log::getCoreLogger()->error(__VA_ARGS__);                                                                                                   \
            wasLogged = true;                                                                                                                                  \
        }                                                                                                                                                      \
    } while (0)
#define AX_CORE_LOG_ERROR(...)                                                                                                                                 \
    do {                                                                                                                                                       \
        if (Axiom::Log::getCoreLogger()->isEnabled(Axiom::Logger::Priority::Error)) {                                                                          \
            Axiom::Log::getCoreLogger()->error(__VA_ARGS__);                                                                                                   \
        }                                                                                                                                                      \
    } while (0)

#define AX_LOG_TRACE(...)                                                                                                                                      \
    do {                                                                                                                                                       \
        if (Axiom::Log::getClientLogger()->isEnabled(Axiom::Logger::Priority::Trace)) {                                                                        \
            Axiom::Log::getClientLogger()->trace(__VA_ARGS__);                                                                                                 \
        }                                                                                                                                                      \
    } while (0)
#ifdef AX_DEBUG
#define AX_LOG_DEBUG(...)                                                                                                                                      \
    do {                                                                                                                                                       \
        if (Axiom::Log::getClientLogger()->isEnabled(Axiom::Logger::Priority::Debug)) {                                                                        \
            Axiom::Log::getClientLogger()->debug(__VA_ARGS__);                                                                                                 \
        }                                                                                                                                                      \
    } while (0)
#else
#define AX_LOG_DEBUG(...)
#endif // AX_DEBUG

#define AX_LOG_INFO(...)                                                                                                                                       \
    do {                                                                                                                                                       \
        if (Axiom::Log::getClientLogger()->isEnabled(Axiom::Logger::Priority::Info)) {                                                                         \
            Axiom::Log::getClientLogger()->info(__VA_ARGS__);                                                                                                  \
        }                                                                                                                                                      \
    } while (0)
#define AX_LOG_WARN(...)                                                                                                                                       \
    do {                                                                                                                                                       \
        if (Axiom::Log::getClientLogger()->isEnabled(Axiom::Logger::Priority::Warn)) {                                                                         \
            Axiom::Log::getClientLogger()->warn(__VA_ARGS__);                                                                                                  \
        }                                                                                                                                                      \
    } while (0)
#define AX_LOG_ERROR(...)                                                                                                                                      \
    do {                                                                                                                                                       \
        if (Axiom::Log::getClientLogger()->isEnabled(Axiom::Logger::Priority::Error)) {                                                                        \
            Axiom::Log::getClientLogger()->error(__VA_ARGS__);                                                                                                 \
        }                                                                                                                                                      \
    } while (0)

} // namespace Axiom
