#include "axpch.h"
#include "Log.h"

namespace Axiom {
	std::shared_ptr<Logger> Log::coreLogger = std::make_shared<Logger>();
	std::shared_ptr<Logger> Log::clientLogger = std::make_shared<Logger>();

    void Log::init() {
        coreLogger = std::make_shared<Logger>();
        coreLogger->setPriority(Logger::Priority::Trace);
        coreLogger->setInitialString("[CORE] ");
        coreLogger->setTimestampFormat("%Y-%m-%d %H:%M:%S");

        clientLogger = std::make_shared<Logger>();
        clientLogger->setPriority(Logger::Priority::Trace);
        clientLogger->setInitialString("[APP]  ");
        clientLogger->setTimestampFormat("%Y-%m-%d %H:%M:%S");
    }
}
