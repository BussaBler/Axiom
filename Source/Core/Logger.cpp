#include "axpch.h"
#include "Logger.h"

namespace Axiom {
	Logger::Logger() : mPriority(Logger::Priority::Debug), mInitialString("") {}

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

	const std::string Logger::messagePriorityToString(Priority p) {
		std::string priorityString;

		switch (p) {
			case Axiom::Logger::Priority::Trace:
				priorityString = "[TRACE]";
				break;
			case Axiom::Logger::Priority::Debug:
				priorityString = "[DEBUG]";
				break;
			case Axiom::Logger::Priority::Info:
				priorityString = "[INFO]";
				break;
			case Axiom::Logger::Priority::Warn:
				priorityString = "[WARN]";
				break;
			case Axiom::Logger::Priority::Error:
				priorityString = "[ERROR]";
				break;
			case Axiom::Logger::Priority::Count:
				priorityString = "[COUNT]";
				break;
			default:
				priorityString = "[PRIORITY]";
				break;
		}

		return priorityString;
	}
}
