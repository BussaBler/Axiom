#include "axpch.h"

#include "TaskManager.h"

namespace Axiom {
    void TaskManager::submitToMain(std::function<void()> task) {
        std::scoped_lock lock(mainThreadMutex);
        tasksQueue.push_back(std::move(task));
    }

    void TaskManager::executeTasks() {
        std::scoped_lock lock(mainThreadMutex);
        for (const auto& task : tasksQueue) {
            task();
        }
        tasksQueue.clear();
    }
} // namespace Axiom
