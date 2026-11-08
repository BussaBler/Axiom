#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace Axiom {
    class TaskManager {
        friend class Application;

      public:
        TaskManager();
        ~TaskManager();

        void submitToMain(std::function<void()> task);
        void submitToWorker(std::function<void()> job);

      private:
        void executeTasks();

      private:
        std::mutex mainThreadMutex;
        std::vector<std::function<void()>> mainTasksQueue;

        std::vector<std::jthread> workers;
        std::queue<std::function<void()>> workerJobQueue;
        std::mutex workerMutex;
        std::condition_variable_any workerConditionVariable;
    };
} // namespace Axiom
