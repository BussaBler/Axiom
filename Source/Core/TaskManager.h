#pragma once

#include <functional>
#include <mutex>
#include <vector>

namespace Axiom {
    class TaskManager {
        friend class Application;

      public:
        TaskManager() = default;
        ~TaskManager() = default;

        void submitToMain(std::function<void()> task);

      private:
        void executeTasks();

      private:
        std::mutex mainThreadMutex;
        std::vector<std::function<void()>> tasksQueue;
    };
} // namespace Axiom
