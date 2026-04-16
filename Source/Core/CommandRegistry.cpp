#include "CommandRegistry.h"

namespace Axiom {
    std::unordered_map<std::string, CommandRegistry::CommandCallback> CommandRegistry::registry = {
        {"clear", [](const std::vector<std::string>& args) { CommandRegistry::commandHistory.clear(); }},
        {"help",
         [](const std::vector<std::string>& args) {
             for (const auto& pair : CommandRegistry::getRegistry()) {
                 CommandRegistry::log("- " + pair.first);
             }
             CommandRegistry::log("Available commands:");
         }},
        {"echo", [](const std::vector<std::string>& args) {
             std::string output;
             for (const auto& arg : args) {
                 output += arg + " ";
             }
             CommandRegistry::log(output);
         }}};
    std::deque<std::string> CommandRegistry::commandHistory;

    void CommandRegistry::registerCommand(const std::string& command, CommandCallback callback) {
        registry[command] = callback;
    }

    void CommandRegistry::executeCommand(const std::string& commandLine) {
        std::istringstream iss(commandLine);
        std::string command;
        iss >> command;

        std::vector<std::string> args;
        std::string arg;
        while (iss >> arg) {
            args.push_back(arg);
        }

        auto it = registry.find(command);
        if (it != registry.end()) {
            it->second(args);
        } else {
            log("Unknown command: " + command);
        }
    }

    void CommandRegistry::log(const std::string& message) {
        commandHistory.push_back(message);
        if (commandHistory.size() > 100) {
            commandHistory.pop_front();
        }
    }
} // namespace Axiom