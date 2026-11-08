#include "axpch.h"

#include "TaskManager.h"

#include <cstdint>
#include <functional>
#include <mutex>
#include <stop_token>
#include <thread>
#include <utility>

namespace Axiom {
    TaskManager::TaskManager() {
        uint32_t threadCount = std::thread::hardware_concurrency();
        threadCount = threadCount > 1 ? threadCount - 1 : 1;

        for (uint32_t i = 0; i < threadCount; i++) {
            workers.emplace_back([this](std::stop_token stopToken) {
                if (!stopToken.stop_requested()) {
                    std::function<void()> job;
                    {
                        std::unique_lock<std::mutex> lock(workerMutex);
                        bool hasJob = workerConditionVariable.wait(lock, stopToken, [this]() { return !workerJobQueue.empty(); });
                        if (!hasJob) {
                            return;
                        }

                        job = std::move(workerJobQueue.front());
                        workerJobQueue.pop();
                    }
                    if (job) {
                        job();
                    }
                }
            });
        }
    }

    TaskManager::~TaskManager() {
        for (auto& worker : workers) {
            worker.request_stop();
        }
        workerConditionVariable.notify_all();
    }

    void TaskManager::submitToMain(std::function<void()> task) {
        std::scoped_lock lock(mainThreadMutex);
        mainTasksQueue.push_back(std::move(task));
    }

    void TaskManager::submitToWorker(std::function<void()> job) {
        {
            std::lock_guard<std::mutex> lock(workerMutex);
            workerJobQueue.push(std::move(job));
        }
        workerConditionVariable.notify_one();
    }

    void TaskManager::executeTasks() {
        std::vector<std::function<void()>> currentTasks;
        {
            std::lock_guard<std::mutex> lock(mainThreadMutex);
            currentTasks = std::move(mainTasksQueue);
        }

        for (auto& task : currentTasks) {
            task();
        }
    }
} // namespace Axiom
