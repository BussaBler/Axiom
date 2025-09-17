#pragma once
#include "axpch.h"

namespace Axiom {
	class Logger {
	public:
		enum class Priority {
			Trace = 0,
			Debug,
			Info, 
			Warn,
			Error,
			Count
		};

		Logger();
		~Logger() = default;
		Logger(const Logger&) = delete;
		Logger& operator=(const Logger&) = delete;

		void setPriority(Priority priority) { mPriority = priority; }
		Priority getPriority() const { return mPriority; }

		void setInitialString(const std::string& initial) { mInitialString = initial; }
		const std::string& getInitialString() const { return mInitialString; }

		bool outputToFile(const std::filesystem::path& path = "Logs/log.txt") {
			std::scoped_lock lock(mLogLock);
			mFilePath = path;
			return enableFileOutput();
		}
		const std::filesystem::path& getFilePath() const { return mFilePath; }

		void setTimestampFormat(const std::string& format) { mTimestampFormat = format; }
		const std::string& getTimestampFormat() const { return mTimestampFormat; }

		template<typename... Args> void trace(std::string_view fmt, Args&&... args) { log(Priority::Trace, fmt, std::forward<Args>(args)...); }
		template<typename... Args> void debug(std::string_view fmt, Args&&... args) { log(Priority::Debug, fmt, std::forward<Args>(args)...); }
		template<typename... Args> void info(std::string_view fmt, Args&&... args) { log(Priority::Info, fmt, std::forward<Args>(args)...); }
		template<typename... Args> void warn(std::string_view fmt, Args&&... args) { log(Priority::Warn, fmt, std::forward<Args>(args)...); }
		template<typename... Args> void error(std::string_view fmt, Args&&... args) { log(Priority::Error, fmt, std::forward<Args>(args)...); }

	private:
		bool enableFileOutput();
		const std::string messagePriorityToString(Priority p);

		template<typename... Args>
		void log(Priority priority, std::string_view fmtString, Args&&... args) {
			if (priority < mPriority) {
				return;
			}

			auto now = std::chrono::system_clock::now();
			auto timeToNow = std::chrono::system_clock::to_time_t(now);
			std::tm tmNow;
#if defined(_MSC_VER)
			localtime_s(&tmNow, &timeToNow);
#else
			localtime_r(&timeToNow, &tmNow);
#endif

			char timeBuffer[64];
			std::strftime(timeBuffer, sizeof(timeBuffer), mTimestampFormat.c_str(), &tmNow);

			auto fmt_args = std::make_format_args<std::format_context>(static_cast<const Args&>(args)...);
			std::string formatted = std::vformat(fmtString, fmt_args);

			std::scoped_lock lock(mLogLock);
			auto color = mColorCodes[static_cast<size_t>(priority)];
			auto reset = mColorCodes[static_cast<size_t>(Priority::Trace)];
			
			std::cout << color << '[' << timeBuffer << ']' << messagePriorityToString(priority) << mInitialString << formatted << '\n' << reset;

			if (mFile.is_open()) {
				mFile << '[' << timeBuffer << ']' << messagePriorityToString(priority) << mInitialString << formatted << '\n';
			}
		}

	private:
		Priority mPriority;
		std::string mInitialString;
		std::string mTimestampFormat;
		std::filesystem::path mFilePath;
		std::ofstream mFile;
		std::mutex mLogLock;
		
		static constexpr std::array<std::string_view, static_cast<size_t>(Priority::Count)> mColorCodes{
			 "\033[37m", // Trace: White
			 "\033[36m", // Debug: Cyan
			 "\033[32m", // Info: Green
			 "\033[33m", // Warn: Yellow
			 "\033[31m"  // Error: Red
		};
	};
}

