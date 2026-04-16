#pragma once
#include "axpch.h"

namespace Axiom {
    class CommandRegistry {
      public:
        using CommandCallback = std::function<void(const std::vector<std::string>& args)>;
        static void registerCommand(const std::string& command, CommandCallback callback);
        static void executeCommand(const std::string& commandLine);
        static void log(const std::string& message);
        inline static const std::unordered_map<std::string, CommandCallback>& getRegistry() { return registry; }
        inline static const std::deque<std::string>& getCommandHistory() { return commandHistory; }

      private:
        static std::unordered_map<std::string, CommandCallback> registry;
        static std::deque<std::string> commandHistory;
    };
} // namespace Axiom