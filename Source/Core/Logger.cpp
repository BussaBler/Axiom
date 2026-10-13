#include "axpch.h"

#include "Logger.h"

#include <string_view>

namespace Axiom {
    Logger::Logger() : mPriority(Logger::Priority::Debug), mInitialString("") {
    }

    Logger::~Logger() {
        if (mFile.is_open()) {
            mFile.close();
        }
    }

    bool Logger::enableFileOutput() {
        mFile.clear();

        std::filesystem::path parentDir = mFilePath.parent_path();

        if (!parentDir.empty() && !std::filesystem::exists(parentDir)) {
            std::filesystem::create_directories(parentDir);
        }
        mFile.open(mFilePath, std::ios::app);

        return mFile.is_open();
    }

    const std::string_view Logger::messagePriorityToString(Priority p) {

        switch (p) {
        case Axiom::Logger::Priority::Trace:
            return "[TRACE]";
        case Axiom::Logger::Priority::Debug:
            return "[DEBUG]";
        case Axiom::Logger::Priority::Info:
            return "[INFO]";
        case Axiom::Logger::Priority::Warn:
            return "[WARN]";
        case Axiom::Logger::Priority::Error:
            return "[ERROR]";
        case Axiom::Logger::Priority::Count:
            return "[COUNT]";
        default:
            return "[PRIORITY]";
        }
    }
} // namespace Axiom
